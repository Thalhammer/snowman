#include <cmath>
#include <frame-info.h>
#include <limits>
#include <mfcc-stream.h>
#include <snowboy-options.h>
#include <vector-wrapper.h>

namespace snowboy {
	void MfccStreamOptions::Register(const std::string& prefix, OptionsItf* opts) {
		mel_filter.Register(prefix, opts);
		opts->Register(prefix, "num-cepstral-coeffs", "Number of cepstral coefficients.", &num_cepstral_coeffs);
		opts->Register(prefix, "use-energy", "If true, replace C0 with log energy.", &use_energy);
		opts->Register(prefix, "cepstral-lifter", "Cepstral lifter coefficient.", &cepstral_lifter);
	}

	MfccStream::MfccStream(const MfccStreamOptions& options) {
		m_options = options;
		m_num_fft_points = 0;
		field_x48 = 0.0f;
		// TODO: Can we optimize this matrix ?
		Matrix m;
		m.Resize(m_options.mel_filter.num_bins, m_options.mel_filter.num_bins);
		ComputeDctMatrixTypeIII(m);
		m_cepstral_coeffs.Resize(m_options.num_cepstral_coeffs, MatrixResizeType::kUndefined);
		ComputeCepstralLifterCoeffs(m_options.cepstral_lifter, &m_cepstral_coeffs);
		m_dct_matrix.Resize(m_options.num_cepstral_coeffs, m_options.mel_filter.num_bins);
		m_dct_matrix.CopyFromMat(m.RowRange(0, m_options.num_cepstral_coeffs), MatrixTransposeType::kNoTrans);
	}

	int MfccStream::Read(Matrix* mat, std::vector<FrameInfo>* info) {
		Matrix m;
		auto res = m_connectedStream->Read(&m, info);
		if ((res & 0xc2) != 0 || m.m_rows == 0) {
			mat->Resize(0, 0);
			info->clear();
			return res;
		}
		SNOWBOY_ASSERT(!m.HasNan() && !m.HasInfinity());
		if (m_num_fft_points != m.cols()) {
			InitMelFilterBank(m.cols());
		}
		SNOWBOY_ASSERT(m_num_fft_points == m.cols());
		mat->Resize(m.rows(), m_options.num_cepstral_coeffs);
		for (size_t r = 0; r < m.rows(); r++) {
			SubVector svec{m, r};
			float energy = 0.0f;
			if (m_options.use_energy) {
				float f = svec.DotVec(svec);
				f = std::max(std::numeric_limits<float>::min(), f);
				f = logf(f) - field_x48;
				energy = f;
			}
			SubVector svec_out{*mat, r};
			// TODO: Couldn't we skip ComputeMfcc if we overwrite it in the next step ?
			ComputeMfcc(svec, &svec_out);
			if (m_options.use_energy) {
				svec_out[0] = energy;
			}
		}
		return res;
	}

	bool MfccStream::Reset() {
		m_melfilterbank.reset();
		m_num_fft_points = 0;
		field_x48 = 0.0;
		return true;
	}

	std::string MfccStream::Name() const {
		return "MfccStream";
	}

	MfccStream::~MfccStream() {}

	void MfccStream::InitMelFilterBank(size_t num_fft_points) {
		auto options = m_options.mel_filter;
		options.num_fft_points = num_fft_points;
		m_melfilterbank.reset(new MelFilterBank(options));
		m_num_fft_points = num_fft_points;
		field_x48 = logf(static_cast<float>(m_num_fft_points) * 0.5f);
	}

	void MfccStream::ComputeMfcc(const VectorBase& param_1, SubVector* param_2) const {
		// Note: We reuse the space inside param_1 as the result, which means the vector is clobbered afterwards.
		auto power_spectrum = param_1.Range(0, param_1.size() / 2);
		ComputePowerSpectrumReal(param_1, power_spectrum);
		// We normaly have 40 bins, but lets set the size to 128 in case some models use more (highly doubt it)
		FixedVector<128> vout{m_melfilterbank->get_options().num_bins, MatrixResizeType::kUndefined};
		m_melfilterbank->ComputeMelFilterBankEnergy(power_spectrum, vout);
		vout.ApplyFloor(std::numeric_limits<float>::min());
		vout.ApplyLog();
		param_2->AddMatVec(1.0, m_dct_matrix, MatrixTransposeType::kNoTrans, vout, 0.0);
		if (m_options.cepstral_lifter != 0.0) { // TODO: Comparing floats for equality is bad...
			param_2->MulElements(m_cepstral_coeffs);
		}
	}
} // namespace snowboy
