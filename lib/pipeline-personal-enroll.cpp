#include <fft-stream.h>
#include <framer-stream.h>
#include <intercept-stream.h>
#include <mfcc-stream.h>
#include <nnet-stream.h>
#include <pipeline-personal-enroll.h>
#include <snowboy-error.h>
#include <snowboy-options.h>
#include <template-enroll-stream.h>

namespace snowboy {
	void PipelinePersonalEnrollOptions::Register(const std::string& prefix, OptionsItf* opts) {
		opts->Register(prefix, "sample-rate", "Sampling rate.", &sample_rate);
	}

	void PipelinePersonalEnroll::RegisterOptions(const std::string& p, OptionsItf* opts) {
		if (m_isInitialized)
			throw snowboy_exception{"pipeline has already been initialized, you have to call RegisterOptions() before Init()."};

		auto prefix = p;
		if (!prefix.empty()) prefix += ".";

		m_pipelinePersonalEnrollOptions.Register(p, opts);
		m_framerStreamOptions->Register(prefix + "framer", opts);
		m_fftStreamOptions->Register(prefix + "fft", opts);
		m_mfccStreamOptions->Register(prefix + "mfcc", opts);
		m_nnetStreamOptions->Register(prefix + "feat", opts);
		m_templateEnrollStreamOptions->Register(prefix + "enroll", opts);

		opts->Remove(p, "framer.sample-rate");
		opts->Remove(p, "mfcc.sample-rate");
	}

	int PipelinePersonalEnroll::GetPipelineSampleRate() const {
		return m_pipelinePersonalEnrollOptions.sample_rate;
	}

	bool PipelinePersonalEnroll::Init() {
		if (m_isInitialized)
			throw snowboy_exception{"class has already been initialized."};
		m_framerStreamOptions->sample_rate = m_pipelinePersonalEnrollOptions.sample_rate;
		m_mfccStreamOptions->mel_filter.sample_rate = m_pipelinePersonalEnrollOptions.sample_rate;

		m_interceptStream.reset(new InterceptStream{});
		m_framerStream.reset(new FramerStream{*m_framerStreamOptions});
		m_fftStream.reset(new FftStream{*m_fftStreamOptions});
		m_mfccStream.reset(new MfccStream{*m_mfccStreamOptions});
		m_nnetStream.reset(new NnetStream{*m_nnetStreamOptions});
		m_templateEnrollStream.reset(new TemplateEnrollStream{*m_templateEnrollStreamOptions});
		m_framerStream->Connect(m_interceptStream.get());
		m_fftStream->Connect(m_framerStream.get());
		m_mfccStream->Connect(m_fftStream.get());
		m_nnetStream->Connect(m_mfccStream.get());
		m_templateEnrollStream->Connect(m_nnetStream.get());

		m_isInitialized = true;
		return true;
	}

	bool PipelinePersonalEnroll::Reset() {
		if (m_isInitialized) {
			m_interceptStream->Reset();
			m_framerStream->Reset();
			m_fftStream->Reset();
			m_mfccStream->Reset();
			m_nnetStream->Reset();
			m_templateEnrollStream->Reset();
		}
		return true;
	}

	std::string PipelinePersonalEnroll::Name() const {
		return "PipelinePersonalEnroll";
	}

	std::string PipelinePersonalEnroll::OptionPrefix() const {
		return "enrollp";
	}

	PipelinePersonalEnroll::~PipelinePersonalEnroll() {}

	PipelinePersonalEnroll::PipelinePersonalEnroll(const PipelinePersonalEnrollOptions& options)
		: m_pipelinePersonalEnrollOptions{options} {
		m_framerStreamOptions.reset(new FramerStreamOptions{});
		m_framerStreamOptions->sample_rate = 16000;
		m_framerStreamOptions->frame_length_ms = 25;
		m_framerStreamOptions->window_type = "povey";
		m_framerStreamOptions->frame_shift_ms = 10;
		m_framerStreamOptions->dither_coeff = 1.0f;
		m_framerStreamOptions->preemphasis_coeff = 0.97f;
		m_framerStreamOptions->subtract_mean = true;
		m_fftStreamOptions.reset(new FftStreamOptions{});
		m_fftStreamOptions->num_fft_points = -1;
		// FFT Stream used in training seems to only have num_fft_points
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
		m_nnetStreamOptions.reset(new NnetStreamOptions{});
		m_nnetStreamOptions->pad_context = true;
		m_nnetStreamOptions->model_filename = "";
		m_templateEnrollStreamOptions.reset(new TemplateEnrollStreamOptions{});
		m_templateEnrollStreamOptions->combine_distance_metric = "euclidean";
		m_templateEnrollStreamOptions->model_filename = "";
		m_templateEnrollStreamOptions->num_templates = 3;
		m_templateEnrollStreamOptions->min_template_length = 20;
		m_templateEnrollStreamOptions->max_template_length = 500;
	}

	int PipelinePersonalEnroll::RunEnrollment(const MatrixBase& data) {
		if (m_isInitialized) {
			std::vector<FrameInfo> info;
			info.resize(data.m_rows);
			m_interceptStream->SetData(data, info, SnowboySignal(8));
			return m_templateEnrollStream->Read(nullptr, nullptr);
		}
		return 2;
	}

	void PipelinePersonalEnroll::SetModelFilename(const std::string& filename) {
		if (m_isInitialized) {
			m_templateEnrollStream->SetModelFilename(filename);
		} else {
			m_templateEnrollStreamOptions->model_filename = filename;
		}
	}

	int PipelinePersonalEnroll::GetNumTemplates() const {
		if (!m_isInitialized)
			throw snowboy_exception{"pipeline has not been initialized, you have to call GetNumTemplates() after Init()."};
		return m_templateEnrollStream->m_options.num_templates;
	}

} // namespace snowboy
