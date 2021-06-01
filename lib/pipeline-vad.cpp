#include <eavesdrop-stream.h>
#include <fft-stream.h>
#include <framer-stream.h>
#include <frontend-stream.h>
#include <gain-control-stream.h>
#include <intercept-stream.h>
#include <license-lib.h>
#include <mfcc-stream.h>
#include <nnet-stream.h>
#include <pipeline-vad.h>
#include <raw-energy-vad-stream.h>
#include <raw-nnet-vad-stream.h>
#include <snowboy-error.h>
#include <snowboy-io.h>
#include <snowboy-options.h>
#include <template-detect-stream.h>
#include <universal-detect-stream.h>
#include <vad-state-stream.h>

namespace snowboy {

	void PipelineVadOptions::Register(const std::string& prefix, OptionsItf* opts) {
		opts->Register(prefix, "sample-rate", "Sampling rate.", &sampleRate);
		opts->Register(prefix, "apply-frontend", "If true, apply VQE frontend.", &applyFrontend);
	}

	void PipelineVad::RegisterOptions(const std::string& p, OptionsItf* opts) {
		if (m_isInitialized)
			throw snowboy_exception{"pipeline has already been initialized, you have to call RegisterOptions() before Init()."};

		auto prefix = p;
		if (!prefix.empty()) prefix += ".";

		m_pipelineVadOptions.Register(p, opts);
		m_gainControlStreamOptions->Register(prefix + "gc", opts);
		m_frontendStreamOptions->Register(prefix + "frontend", opts);
		m_framerStreamOptions->Register(prefix + "framer", opts);
		m_rawEnergyVadStreamOptions->Register(prefix + "vadr1", opts);
		m_vadStateStreamOptions->Register(prefix + "vads1", opts);
		m_fftStreamOptions->Register(prefix + "fft", opts);
		m_mfccStreamOptions->Register(prefix + "mfcc", opts);
		m_rawNnetVadStreamOptions->Register(prefix + "vadr2", opts);
		m_vadStateStream2Options->Register(prefix + "vads2", opts);

		opts->Remove(p, "framer.sample-rate");
		opts->Remove(p, "mfcc.sample-rate");
	}

	int PipelineVad::GetPipelineSampleRate() const {
		return m_pipelineVadOptions.sampleRate;
	}

	bool PipelineVad::Init() {
		if (m_isInitialized)
			throw snowboy_exception{"class has already been initialized."};

		m_framerStreamOptions->sample_rate = m_pipelineVadOptions.sampleRate;
		m_mfccStreamOptions->mel_filter.sample_rate = m_pipelineVadOptions.sampleRate;
		field_xd1 = m_pipelineVadOptions.applyFrontend;
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

		m_gainControlStream->Connect(m_interceptStream.get());
		if (!field_xd1) {
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
		m_vadStateStream2->field_x2c = 2;
		m_isInitialized = true;
		return true;
	}

	bool PipelineVad::Reset() {
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
		}
		m_eavesdropStreamFrameInfoVector.clear();
		field_xd0 = true;
		return true;
	}

	std::string PipelineVad::Name() const {
		return "PipelineVad";
	}

	std::string PipelineVad::OptionPrefix() const {
		return "vadp";
	}

	PipelineVad::~PipelineVad() {}

	PipelineVad::PipelineVad(const PipelineVadOptions& options) {
		m_pipelineVadOptions = options;
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
		m_eavesdropStreamFrameInfoVector.clear();
		field_xd0 = true;
		field_xd1 = m_pipelineVadOptions.applyFrontend;
	}

	void PipelineVad::ApplyFrontend(bool apply) {
		if (m_isInitialized == false) {
			m_pipelineVadOptions.applyFrontend = apply;
			field_xd1 = apply;
			return;
		}
		if (apply != field_xd1) {
			field_xd1 = apply;
			if (apply == false) {
				m_framerStream->Connect(m_gainControlStream.get());
			} else {
				m_frontendStream->Connect(m_gainControlStream.get());
				m_framerStream->Connect(m_frontendStream.get());
			}
		}
	}

	int PipelineVad::RunVad(const MatrixBase& data, bool is_end) {
		if (!m_isInitialized)
			throw snowboy_exception{"pipeline has not been initialized yet."};

		std::vector<FrameInfo> info;
		info.resize(data.m_rows);
		m_interceptStream->SetData(data, info, static_cast<SnowboySignal>(is_end ? 0x30 : 0x20));
		do {
			Matrix tmat;
			std::vector<FrameInfo> tinfo;
			auto tres = m_vadStateStream2->Read(&tmat, &tinfo);
			m_rawEnergyVadStream->UpdateBackgroundEnergy(m_eavesdropStreamFrameInfoVector);
			m_eavesdropStreamFrameInfoVector.clear();
			if ((tres & 4) != 0) {
				field_xd0 = false;
			}
			if ((tres & 8) != 0) {
				field_xd0 = true;
			}
			if ((tres & 0x20) != 0) break;
		} while (true);
		return this->field_xd0 ? -2 : 0;
	}

	void PipelineVad::SetAudioGain(float gain) {
		if (!m_isInitialized)
			throw snowboy_exception{"pipeline has not been initialized yet."};
		m_gainControlStream->SetAudioGain(gain);
	}

	void PipelineVad::SetMaxAudioAmplitude(float maxAmplitude) {
		if (!m_isInitialized)
			throw snowboy_exception{"pipeline has not been initialized yet."};
		m_gainControlStream->SetMaxAudioAmplitude(maxAmplitude);
	}

} // namespace snowboy
