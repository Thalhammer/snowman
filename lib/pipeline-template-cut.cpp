#include <algorithm>
#include <cassert>
#include <cmath>
#include <eavesdrop-stream.h>
#include <fft-stream.h>
#include <framer-stream.h>
#include <intercept-stream.h>
#include <limits>
#include <mfcc-stream.h>
#include <nnet-stream.h>
#include <pipeline-template-cut.h>
#include <raw-nnet-vad-stream.h>
#include <snowboy-error.h>
#include <snowboy-options.h>
#include <vad-lib.h>

namespace snowboy {
	void PipelineTemplateCutOptions::Register(const std::string& prefix, OptionsItf* opts) {
		opts->Register(prefix, "sample-rate", "Sampling rate.", &sample_rate);
		opts->Register(prefix, "bg-energy-threshold", "Threshold for energy VAD.", &bg_energy_threshold);
		opts->Register(prefix, "min-non-voice-frames", "Minimal number of non-voice frames to be accumulated before jumping into a non-voice state.", &min_non_voice_frames);
		opts->Register(prefix, "min-voice-frames", "Minimal number of voice frames to be accumulated before jumping into a voice state.", &min_voice_frames);
	}

	void PipelineTemplateCut::RegisterOptions(const std::string& p, OptionsItf* opts) {
		if (m_isInitialized)
			throw snowboy_exception{"pipeline has already been initialized, you have to call RegisterOptions() before Init()."};

		auto prefix = p;
		if (!prefix.empty()) prefix += ".";

		m_pipelineTemplateCutOptions.Register(p, opts);
		m_framerStreamOptions.Register(prefix + "framer", opts);
		m_fftStreamOptions->Register(prefix + "fft", opts);
		m_mfccStreamOptions->Register(prefix + "mfcc", opts);
		m_rawNnetVadStreamOptions->Register(prefix + "vadr", opts);

		opts->Remove(p, "framer.sample-rate");
		opts->Remove(p, "mfcc.sample-rate");
	}

	int PipelineTemplateCut::GetPipelineSampleRate() const {
		return m_pipelineTemplateCutOptions.sample_rate;
	}

	bool PipelineTemplateCut::Init() {
		if (m_isInitialized)
			throw snowboy_exception{"class has already been initialized."};

		m_framerStreamOptions.sample_rate = m_pipelineTemplateCutOptions.sample_rate;
		m_mfccStreamOptions->mel_filter.sample_rate = m_pipelineTemplateCutOptions.sample_rate;

		m_interceptStream.reset(new InterceptStream{});
		m_framerStream.reset(new FramerStream{m_framerStreamOptions});
		m_eavesdropStream.reset(new EavesdropStream{&m_eavesdropMatrix, nullptr});
		m_fftStream.reset(new FftStream{*m_fftStreamOptions});
		m_mfccStream.reset(new MfccStream{*m_mfccStreamOptions});
		m_rawNnetVadStream.reset(new RawNnetVadStream{*m_rawNnetVadStreamOptions});

		m_framerStream->Connect(m_interceptStream.get());
		m_eavesdropStream->Connect(m_framerStream.get());
		m_fftStream->Connect(m_eavesdropStream.get());
		m_mfccStream->Connect(m_fftStream.get());
		m_rawNnetVadStream->Connect(m_mfccStream.get());

		m_isInitialized = true;
		return true;
	}

	bool PipelineTemplateCut::Reset() {
		if (m_isInitialized) {
			m_interceptStream->Reset();
			m_framerStream->Reset();
			m_fftStream->Reset();
			m_mfccStream->Reset();
			m_rawNnetVadStream->Reset();
		}
		m_eavesdropMatrix.Resize(0, 0);
		return true;
	}

	std::string PipelineTemplateCut::Name() const {
		return "PipelineTemplateCut";
	}

	std::string PipelineTemplateCut::OptionPrefix() const {
		return "cutp";
	}

	PipelineTemplateCut::~PipelineTemplateCut() {}

	PipelineTemplateCut::PipelineTemplateCut(const PipelineTemplateCutOptions& options)
		: m_pipelineTemplateCutOptions{options} {
		m_framerStreamOptions.sample_rate = 16000;
		m_framerStreamOptions.frame_length_ms = 25;
		m_framerStreamOptions.window_type = "povey";
		m_framerStreamOptions.frame_shift_ms = 10;
		m_framerStreamOptions.dither_coeff = 1.0f;
		m_framerStreamOptions.preemphasis_coeff = 0.97f;
		m_framerStreamOptions.subtract_mean = true;
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
		m_rawNnetVadStreamOptions.reset(new RawNnetVadStreamOptions{});
		m_rawNnetVadStreamOptions->non_voice_index = 0;
		m_rawNnetVadStreamOptions->non_voice_threshold = 0.4f;
	}

	int PipelineTemplateCut::CutTemplate(const MatrixBase& in, Matrix* out) {
		if (!m_isInitialized)
			throw snowboy_exception{"class has not been initialized."};

		std::vector<FrameInfo> info;
		info.resize(in.m_rows);
		m_interceptStream->SetData(in, info, SnowboySignal(8));
		Matrix read_mat;
		std::vector<FrameInfo> read_info;
		m_rawNnetVadStream->Read(&read_mat, &read_info);
		read_mat.Resize(0, 0);
		if (m_eavesdropMatrix.m_rows == 0) {
			return 0x400;
		} else {
			int boundary_a, boundary_b;
			ComputeTemplateBoundary(m_eavesdropMatrix, read_info, &boundary_a, &boundary_b);
			if (boundary_a == boundary_b) {
				return 0x400;
			} else {
				auto s = m_framerStreamOptions.frame_shift_ms;
				auto l = m_framerStreamOptions.frame_length_ms;
				auto r = m_framerStreamOptions.sample_rate;
				auto iVar6 = (boundary_a * s * r) / 1000.0f;
				auto iVar5 = ((boundary_b * s + l) * r / 1000.0f) - iVar6;
				out->Resize(in.m_rows, iVar5, MatrixResizeType::kUndefined);
				out->CopyFromMat(in.ColRange(iVar6, iVar5), MatrixTransposeType::kNoTrans);
				return 1;
			}
		}
		return 2;
	}

	void PipelineTemplateCut::ComputeTemplateBoundary(const MatrixBase& param_1, const std::vector<FrameInfo>& param_2, int* param_3, int* param_4) const {
		std::vector<float> temp;
		temp.resize(param_1.rows());
		for (size_t row = 0; row < param_1.rows(); row++) {
			auto fVar17 = SubVector{param_1, row}.DotVec(SubVector{param_1, row});
			fVar17 = std::max(fVar17, std::numeric_limits<float>::min());
			fVar17 = logf(fVar17);
			temp[row] = fVar17;
		}

		float avg_voice_energy = 0.0f;
		size_t num_voice_samples = 0;
		std::vector<VoiceType> local_98;
		for (size_t i = 0; i < param_2.size(); i++) {
			if ((param_2[i].flags & 1) == 0) {
				num_voice_samples++;
				avg_voice_energy += temp[i];
			}
		}
		avg_voice_energy /= num_voice_samples;
		local_98.resize(param_2.size());

		for (size_t i = 0; i < param_2.size(); i++) {
			if ((param_2[i].flags & 1) != 0
				|| m_pipelineTemplateCutOptions.bg_energy_threshold <= temp[i] - avg_voice_energy) {
				local_98[i] = VT_1;
			} else {
				local_98[i] = VT_2;
			}
		}

		std::vector<VoiceStateType> piStack88;
		VadStateOptions opts{};
		opts.min_non_voice_frames = m_pipelineTemplateCutOptions.min_non_voice_frames;
		opts.min_voice_frames = m_pipelineTemplateCutOptions.min_voice_frames;
		VadState state{opts};
		*param_3 = -1;
		state.GetVoiceStates(local_98, &piStack88);

		size_t iVar7;
		for (iVar7 = 0; piStack88[iVar7] != 1 && iVar7 != piStack88.size(); iVar7++) {
		}
		if (iVar7 == piStack88.size()) {
			if (*param_3 == -1) {
				*param_3 = 0;
				*param_4 = 0;
				return;
			}
		} else {
			iVar7 -= std::min<size_t>(iVar7, m_pipelineTemplateCutOptions.min_voice_frames);
			*param_3 = iVar7;
		}

		*param_4 = 0;
		std::reverse(local_98.begin(), local_98.end());
		state.Reset();
		state.GetVoiceStates(local_98, &piStack88);
		if (piStack88.size() != 0) {
			size_t iVar7 = 0;
			for (size_t i = 0; i < piStack88.size(); i++) {
				if (piStack88[i] == 1) break;
				iVar7 = i + 1;
			}
			if (iVar7 == piStack88.size()) {
				*param_4 = ((int)piStack88.size() - 1) - *param_4;
			} else {
				iVar7 -= std::min<size_t>(iVar7, m_pipelineTemplateCutOptions.min_voice_frames);
				*param_4 = ((int)piStack88.size() - 1) - iVar7;
			}
		} else {
			*param_4 = ((int)piStack88.size() - 1) - *param_4;
		}
	}
} // namespace snowboy
