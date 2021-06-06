#include <eavesdrop-stream.h>
#include <fft-stream.h>
#include <framer-stream.h>
#include <frontend-stream.h>
#include <gain-control-stream.h>
#include <intercept-stream.h>
#include <license-lib.h>
#include <mfcc-stream.h>
#include <nnet-stream.h>
#include <pipeline-detect.h>
#include <raw-energy-vad-stream.h>
#include <raw-nnet-vad-stream.h>
#include <snowboy-error.h>
#include <snowboy-io.h>
#include <snowboy-options.h>
#include <sstream>
#include <template-detect-stream.h>
#include <universal-detect-stream.h>
#include <vad-state-stream.h>

namespace snowboy {

	void PipelineDetectOptions::Register(const std::string& prefix, OptionsItf* opts) {
		opts->Register(prefix, "sample-rate", "Sampling rate.", &sampleRate);
		opts->Register(prefix, "apply-frontend", "If true, apply VQE frontend.", &applyFrontend);
	}

	void PipelineDetect::RegisterOptions(const std::string& p, OptionsItf* opts) {
		if (m_isInitialized)
			throw snowboy_exception{"pipeline has already been initialized, you have to call RegisterOptions() before Init()"};

		auto prefix = p;
		if (!prefix.empty()) prefix += ".";

		m_pipelineDetectOptions.Register(p, opts);
		m_gainControlStreamOptions->Register(prefix + "gc", opts);
		m_frontendStreamOptions->Register(prefix + "frontend", opts);
		m_framerStreamOptions->Register(prefix + "framer", opts);
		m_rawEnergyVadStreamOptions->Register(prefix + "vadr1", opts);
		m_vadStateStreamOptions->Register(prefix + "vads1", opts);
		m_fftStreamOptions->Register(prefix + "fft", opts);
		m_mfccStreamOptions->Register(prefix + "mfcc", opts);
		m_rawNnetVadStreamOptions->Register(prefix + "vadr2", opts);
		m_vadStateStream2Options->Register(prefix + "vads2", opts);
		m_templateDetectNnetStreamOptions->Register(prefix + "feat", opts);
		m_templateDetectStreamOptions->Register(prefix + "pdetect", opts);
		m_universalDetectStreamOptions->Register(prefix + "udetect", opts);

		opts->Remove(p, "framer.sample-rate");
		opts->Remove(p, "mfcc.sample-rate");
		opts->Remove(p, "pdetect.model-str");
		opts->Remove(p, "pdetect.sensitivity-str");
		opts->Remove(p, "udetect.model-str");
		opts->Remove(p, "udetect.sensitivity-str");
	}

	int PipelineDetect::GetPipelineSampleRate() const {
		return m_pipelineDetectOptions.sampleRate;
	}

	bool PipelineDetect::Init() {
		if (m_isInitialized)
			throw snowboy_exception{"class has already been initialized"};

		if (m_templateDetectStreamOptions->model_str == "" && m_universalDetectStreamOptions->model_str == "")
			throw snowboy_exception{"no model detected! You have to provide at least one personal or one universal model by calling SetModel()"};

		m_framerStreamOptions->sample_rate = m_pipelineDetectOptions.sampleRate;
		m_mfccStreamOptions->mel_filter.sample_rate = m_pipelineDetectOptions.sampleRate;
		m_frontend_enabled = m_pipelineDetectOptions.applyFrontend;
		m_interceptStream.reset(new InterceptStream{});
		m_gainControlStream.reset(new GainControlStream{*m_gainControlStreamOptions});
		m_frontendStream.reset(new FrontendStream{*m_frontendStreamOptions});
		m_framerStream.reset(new FramerStream{*m_framerStreamOptions});
		m_rawEnergyVadStream.reset(new RawEnergyVadStream{*m_rawEnergyVadStreamOptions});
		m_vadStateStream.reset(new VadStateStream{*m_vadStateStreamOptions});
		m_fftStream.reset(new FftStream{*m_fftStreamOptions});
		m_mfccStream.reset(new MfccStream{*m_mfccStreamOptions});
		m_rawNnetVadStream.reset(new RawNnetVadStream{*m_rawNnetVadStreamOptions});
		m_eavesdropStream.reset(new EavesdropStream{nullptr, &m_eavesdropStreamFrameInfoVector});
		m_vadStateStream2.reset(new VadStateStream{*m_vadStateStream2Options});
		if (m_templateDetectStreamOptions->model_str != "") {
			m_templateDetectInterceptStream.reset(new InterceptStream{});
			m_templateDetectNnetStream.reset(new NnetStream{*m_templateDetectNnetStreamOptions});
			m_templateDetectStream.reset(new TemplateDetectStream{*m_templateDetectStreamOptions});
		}
		if (m_universalDetectStreamOptions->model_str != "") {
			m_universalDetectInterceptStream.reset(new InterceptStream{});
			m_universalDetectStream.reset(new UniversalDetectStream{*m_universalDetectStreamOptions});
		}
		m_gainControlStream->Connect(m_interceptStream.get());
		if (!m_frontend_enabled) {
			m_framerStream->Connect(m_gainControlStream.get());
		} else {
			m_frontendStream->Connect(m_gainControlStream.get());
			m_framerStream->Connect(m_frontendStream.get());
		}
		m_rawEnergyVadStream->Connect(m_framerStream.get());
		m_vadStateStream->Connect(m_rawEnergyVadStream.get());
		m_fftStream->Connect(m_vadStateStream.get());
		m_mfccStream->Connect(m_fftStream.get());
		m_rawNnetVadStream->Connect(m_mfccStream.get());
		m_eavesdropStream->Connect(m_rawNnetVadStream.get());
		m_vadStateStream2->Connect(m_eavesdropStream.get());
		m_vadStateStream->field_x2c = 1;
		m_vadStateStream->field_x2c = 2;
		if (m_templateDetectStream) {
			m_templateDetectNnetStream->Connect(m_templateDetectInterceptStream.get());
			m_templateDetectStream->Connect(m_templateDetectNnetStream.get());
		}
		if (m_universalDetectStream) {
			m_universalDetectStream->Connect(m_universalDetectInterceptStream.get());
		}
		int npersonal = 0;
		int nuniversal = 0;
		int kwid = 1;
		for (size_t i = 0; i < m_is_personal_model.size(); i++) {
			if (m_is_personal_model[i] == false) {
				for (size_t x = 0; x < m_universalDetectStream->NumHotwords(nuniversal); x++) {
					m_universal_kw_mapping.push_back(kwid);
					kwid++;
				}
				nuniversal++;
			} else {
				for (size_t x = 0; x < m_templateDetectStream->NumHotwords(npersonal); x++) {
					m_personal_kw_mapping.push_back(kwid);
					kwid++;
				}
				npersonal++;
			}
		}
		m_isInitialized = true;
		return true;
	}

	bool PipelineDetect::Reset() {
		CheckSnowboyLicense();
		if (m_isInitialized) {
			m_interceptStream->Reset();
			m_gainControlStream->Reset();
			m_frontendStream->Reset();
			m_framerStream->Reset();
			m_rawEnergyVadStream->Reset();
			m_vadStateStream->Reset();
			m_fftStream->Reset();
			m_mfccStream->Reset();
			m_rawNnetVadStream->Reset();
			m_eavesdropStream->Reset();
			m_vadStateStream2->Reset();
			if (m_templateDetectStream) {
				m_templateDetectInterceptStream->Reset();
				m_templateDetectNnetStream->Reset();
				m_templateDetectStream->Reset();
			}
			if (m_universalDetectStream) {
				m_universalDetectInterceptStream->Reset();
				m_universalDetectStream->Reset();
			}
		}
		m_eavesdropStreamFrameInfoVector.clear();
		field_x168 = true;
		return true;
	}

	std::string PipelineDetect::Name() const {
		return "PipelineDetect";
	}

	std::string PipelineDetect::OptionPrefix() const {
		return "detectp";
	}

	PipelineDetect::~PipelineDetect() {}

	PipelineDetect::PipelineDetect(const PipelineDetectOptions& options) {
		m_pipelineDetectOptions = options;
		CheckSnowboyLicense();
		m_gainControlStreamOptions.reset(new GainControlStreamOptions{});
		m_gainControlStreamOptions->m_audioGain = 1.0f;
		m_frontendStreamOptions.reset(new FrontendStreamOptions{});
		m_frontendStreamOptions->ns_power = "1";
		m_frontendStreamOptions->dr_power = "1";
		m_frontendStreamOptions->agc_level = "2";
		m_frontendStreamOptions->agc_power = "12";
		m_framerStreamOptions.reset(new FramerStreamOptions{});
		m_framerStreamOptions->sample_rate = 16000;
		m_framerStreamOptions->frame_length_ms = 25;
		m_framerStreamOptions->window_type = "povey";
		m_framerStreamOptions->frame_shift_ms = 10;
		m_framerStreamOptions->dither_coeff = 1.0f;
		m_framerStreamOptions->preemphasis_coeff = 0.97f;
		m_framerStreamOptions->subtract_mean = true;
		m_rawEnergyVadStreamOptions.reset(new RawEnergyVadStreamOptions{});
		m_rawEnergyVadStreamOptions->init_bg_energy = true;
		m_rawEnergyVadStreamOptions->bg_energy_threshold = 2.0f;
		m_rawEnergyVadStreamOptions->bg_energy_cap = 12.0f;
		m_rawEnergyVadStreamOptions->bg_buffer_size = 60;
		m_rawEnergyVadStreamOptions->raw_buffer_extra = 0;
		m_vadStateStreamOptions.reset(new VadStateStreamOptions{});
		m_vadStateStreamOptions->min_non_voice_frames = 100;
		m_vadStateStreamOptions->min_voice_frames = 10;
		m_vadStateStreamOptions->remove_non_voice = false;
		m_vadStateStreamOptions->extra_frame_adjust = 20;
		m_fftStreamOptions.reset(new FftStreamOptions{});
		m_fftStreamOptions->num_fft_points = -1;
		m_fftStreamOptions->method = "srfft";
		m_mfccStreamOptions.reset(new MfccStreamOptions{});
		m_mfccStreamOptions->mel_filter.num_bins = 23;
		m_mfccStreamOptions->mel_filter.num_fft_points = 512;
		m_mfccStreamOptions->mel_filter.sample_rate = 16000;
		m_mfccStreamOptions->mel_filter.low_frequency = 20.0f;
		m_mfccStreamOptions->mel_filter.high_frequency = 8000.0f;
		m_mfccStreamOptions->mel_filter.vtln_low_frequency = 100.0f;
		m_mfccStreamOptions->mel_filter.vtln_high_frequency = 7500.0f;
		m_mfccStreamOptions->mel_filter.vtln_warping_factor = 1.0f;
		m_mfccStreamOptions->num_cepstral_coeffs = 13;
		m_mfccStreamOptions->use_energy = true;
		m_mfccStreamOptions->cepstral_lifter = 22.0f;
		m_rawNnetVadStreamOptions.reset(new RawNnetVadStreamOptions{});
		m_rawNnetVadStreamOptions->non_voice_index = 0;
		m_rawNnetVadStreamOptions->non_voice_threshold = 0.4f;
		m_rawNnetVadStreamOptions->model_filename = "";
		m_vadStateStream2Options.reset(new VadStateStreamOptions{});
		m_vadStateStream2Options->min_non_voice_frames = 100;
		m_vadStateStream2Options->min_voice_frames = 10;
		m_vadStateStream2Options->remove_non_voice = false;
		m_vadStateStream2Options->extra_frame_adjust = 20;
		m_templateDetectNnetStreamOptions.reset(new NnetStreamOptions{});
		m_templateDetectNnetStreamOptions->pad_context = true;
		m_templateDetectNnetStreamOptions->model_filename = "";
		m_templateDetectStreamOptions.reset(new TemplateDetectStreamOptions{});
		m_templateDetectStreamOptions->dtw_options.band_width = 20;
		m_templateDetectStreamOptions->dtw_options.distance_metric = "euclidean";
		m_templateDetectStreamOptions->model_str = "";
		m_templateDetectStreamOptions->sensitivity_str = "";
		m_templateDetectStreamOptions->slide_step = 1;
		m_universalDetectStreamOptions.reset(new UniversalDetectStreamOptions{});
		m_universalDetectStreamOptions->slide_step = 1;
		m_universalDetectStreamOptions->min_num_frames_per_phone = 3;
		m_universalDetectStreamOptions->sensitivity_str = "";
		m_universalDetectStreamOptions->high_sensitivity_str = "";
		m_universalDetectStreamOptions->model_str = "";
		m_universalDetectStreamOptions->smooth_window_str = "";
		m_universalDetectStreamOptions->slide_window_str = "";
		m_universalDetectStreamOptions->debug_mode = false;
		m_universalDetectStreamOptions->num_repeats = 3;
		m_eavesdropStreamFrameInfoVector.clear();
		field_x168 = true;
		m_frontend_enabled = m_pipelineDetectOptions.applyFrontend;
	}

	void PipelineDetect::ApplyFrontend(bool apply) {
		if (m_isInitialized == false) {
			m_pipelineDetectOptions.applyFrontend = apply;
			m_frontend_enabled = apply;
			return;
		}
		if (apply != m_frontend_enabled) {
			m_frontend_enabled = apply;
			if (apply == false) {
				m_framerStream->Connect(m_gainControlStream.get());
			} else {
				m_frontendStream->Connect(m_gainControlStream.get());
				m_framerStream->Connect(m_frontendStream.get());
			}
		}
	}

	void PipelineDetect::ClassifyModels(const std::string& model_str, std::string* personal_models, std::string* universal_models) {
		personal_models->clear();
		universal_models->clear();
		std::vector<std::string> parts;
		SplitStringToVector(model_str, global_snowboy_string_delimiter, &parts);
		m_is_personal_model.resize(parts.size(), false);
		for (size_t i = 0; i < parts.size(); i++) {
			if (ClassifyModel(parts[i])) {
				if (!personal_models->empty()) personal_models->append(",");
				personal_models->append(parts[i]);
				m_is_personal_model[i] = true;
			} else {
				if (!universal_models->empty()) universal_models->append(",");
				universal_models->append(parts[i]);
				m_is_personal_model[i] = false;
			}
		}
	}

	bool PipelineDetect::ClassifyModel(const std::string& model_filename) {
		Input in{model_filename};
		auto binary = in.is_binary();
		auto is = in.Stream();
		std::string token;
		ReadToken(binary, &token, is);
		if (token == "<PersonalModel>") {
			return true;
		} else if (token == "<UniversalModel>") {
			return false;
		}
		throw snowboy_exception{"undefined model type detected. Most likely you provided the wrong model"};
	}

	void PipelineDetect::ClassifySensitivities(const std::string& param_1, std::string* param_2, std::string* param_3) const {
		param_2->clear();
		param_3->clear();
		std::vector<std::string> parts;
		SplitStringToVector(param_1, global_snowboy_string_delimiter, &parts);
		auto num_personal = m_templateDetectStream == nullptr ? 0 : m_templateDetectStream->m_models.size();
		auto num_universal = 0;
		if (m_universalDetectStream != nullptr
			&& !m_universalDetectStream->m_model_info.empty()
			&& !m_universalDetectStream->m_model_info.back().keywords.empty()) {
			num_universal = m_universalDetectStream->m_model_info.back().keywords.back().hotword_id;
		}
		if (num_universal + num_personal > parts.size()) {
			std::stringstream ss;
			ss << "number of hotwords and number of sensitivities mismatch, expecting sensitivities for "
			   << num_personal << " personal hotwords, and " << num_universal << " universal hotwords, got "
			   << parts.size() << " sensitivities instead";
			throw snowboy_exception{ss.str()};
		}
		for (size_t i = 0; i < m_is_personal_model.size(); i++) {
			if (m_is_personal_model[i]) {
				if (!param_2->empty()) param_2->append(", ");
				param_2->append(parts[i]);
			} else {
				if (!param_3->empty()) param_3->append(", ");
				param_3->append(parts[i]);
			}
		}
	}

	uint64_t PipelineDetect::GetDetectedFrameId() const {
		if (!m_isInitialized)
			throw snowboy_exception{"pipeline has not been initialized yet"};

		int id = 0;
		if (m_universalDetectStream) {
			id = m_universalDetectStream->field_x5c;
		}
		if (m_templateDetectStream) {
			id = std::max(m_templateDetectStream->field_x90, id);
		}
		return id;
	}

	std::string PipelineDetect::GetSensitivity() const {
		if (!m_isInitialized)
			throw snowboy_exception{"pipeline has not been initialized yet"};

		std::vector<std::string> personal;
		std::vector<std::string> universal;
		if (m_templateDetectStream) SplitStringToVector(m_templateDetectStream->GetSensitivity(), global_snowboy_string_delimiter, &personal);
		if (m_universalDetectStream) SplitStringToVector(m_universalDetectStream->GetSensitivity(), global_snowboy_string_delimiter, &universal);
		auto pit = personal.begin();
		auto uit = universal.begin();
		std::string res;
		for (size_t i = 0; i < m_is_personal_model.size(); i++) {
			if (i != 0) res.append(",");
			if (m_is_personal_model[i]) {
				res.append(*pit);
				pit++;
			} else {
				res.append(*uit);
				uit++;
			}
		}
		return res;
	}

	int PipelineDetect::NumHotwords() const {
		if (!m_isInitialized)
			throw snowboy_exception{"pipeline has not been initialized yet"};

		int num_hotwords = 0;
		if (m_templateDetectStream) num_hotwords += m_templateDetectStream->m_models.size();
		if (m_universalDetectStream && !m_universalDetectStream->m_model_info.empty() && !m_universalDetectStream->m_model_info.back().keywords.empty())
			num_hotwords += m_universalDetectStream->m_model_info.back().keywords.back().hotword_id;
		return num_hotwords;
	}

	int PipelineDetect::RunDetection(const MatrixBase& data, bool is_end) {
		if (!m_isInitialized)
			throw snowboy_exception{"pipeline has not been initialized yet"};

		std::vector<FrameInfo> info;
		info.resize(data.m_rows);
		m_interceptStream->SetData(data, info, static_cast<SnowboySignal>(is_end ? 0x30 : 0x20));
		int x = 0;
		while (x == 0) {
			Matrix tmat;
			std::vector<FrameInfo> tinfo;
			auto tres = m_vadStateStream2->Read(&tmat, &tinfo);
			m_rawEnergyVadStream->UpdateBackgroundEnergy(m_eavesdropStreamFrameInfoVector);
			m_eavesdropStreamFrameInfoVector.clear();
			if (m_templateDetectStream) {
				Matrix ptmat;
				std::vector<FrameInfo> ptinfo;
				m_templateDetectInterceptStream->SetData(tmat, tinfo, static_cast<SnowboySignal>(tres));
				x = m_templateDetectStream->Read(&ptmat, &ptinfo);
				if (ptmat.m_rows == 1 && ptmat.m_cols == 1) {
					this->Reset();
					auto f = ptmat.m_data[0] - 1.0f;
					if (f >= 9.223372e+18) f -= 9.223372e+18;
					x = m_personal_kw_mapping[static_cast<int>(f)];
					return x;
				}
			}
			if (m_universalDetectStream) {
				Matrix utmat;
				std::vector<FrameInfo> utinfo;
				m_universalDetectInterceptStream->SetData(tmat, tinfo, static_cast<SnowboySignal>(tres));
				auto utres = m_universalDetectStream->Read(&utmat, &utinfo);
				x |= utres;
				if (utmat.m_rows == 1 && utmat.m_cols == 1) {
					this->Reset();
					auto f = utmat.m_data[0] - 1.0f;
					if (f >= 9.223372e+18) f -= 9.223372e+18;
					x = m_universal_kw_mapping[static_cast<int>(f)];
					return x;
				}
			}
			if ((x & 4) != 0) {
				field_x168 = false;
			}
			if ((x & 8) != 0) {
				field_x168 = true;
			}
			x &= 0x20;
		}
		return this->field_x168 ? -2 : 0;
	}

	void PipelineDetect::SetAudioGain(float gain) {
		if (!m_isInitialized)
			throw snowboy_exception{"pipeline has not been initialized yet"};
		m_gainControlStream->SetAudioGain(gain);
	}

	void PipelineDetect::SetHighSensitivity(const std::string& param_1) {
		if (!m_isInitialized)
			throw snowboy_exception{"pipeline has not been initialized yet"};
		if (!m_universalDetectStream) return;
		std::string personal, universal;
		ClassifySensitivities(param_1, &personal, &universal);
		m_universalDetectStream->SetHighSensitivity(universal);
	}

	void PipelineDetect::SetMaxAudioAmplitude(float maxAmplitude) {
		if (!m_isInitialized)
			throw snowboy_exception{"pipeline has not been initialized yet"};
		m_gainControlStream->SetMaxAudioAmplitude(maxAmplitude);
	}

	void PipelineDetect::SetModel(const std::string& model) {
		if (m_isInitialized)
			throw snowboy_exception{"pipeline has already been initialized, you have to call SetModel() before Init()"};
		ClassifyModels(model, &m_templateDetectStreamOptions->model_str, &m_universalDetectStreamOptions->model_str);
	}

	void PipelineDetect::SetSensitivity(const std::string& param_1) {
		if (!m_isInitialized)
			throw snowboy_exception{"pipeline has not been initialized yet"};
		std::string personal, universal;
		ClassifySensitivities(param_1, &personal, &universal);
		if (m_templateDetectStream) m_templateDetectStream->SetSensitivity(personal);
		if (m_universalDetectStream) m_universalDetectStream->SetSensitivity(universal);
	}

	void PipelineDetect::UpdateModel() const {
		if (!m_isInitialized)
			throw snowboy_exception{"pipeline has not been initialized yet"};
		if (m_templateDetectStream) m_templateDetectStream->UpdateModel();
		if (m_universalDetectStream) m_universalDetectStream->UpdateModel();
	}

} // namespace snowboy
