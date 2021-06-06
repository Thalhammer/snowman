#include <feat-lib.h>
#include <math.h>
#include <matrix-wrapper.h>
#include <snowboy-options.h>

namespace snowboy {
	void MelFilterBankOptions::Register(const std::string& prefix, OptionsItf* opts) {
		opts->Register(prefix, "num-bins", "Number of triangular bins.", &num_bins);
		opts->Register(prefix, "num-fft-points", "Number of FFT points.", &num_fft_points);
		opts->Register(prefix, "sample-rate", "Sampling rate.", &sample_rate);
		opts->Register(prefix, "low-frequency", "Lowest frequency for triangular bins.", &low_frequency);
		opts->Register(prefix, "high-frequency", "Highest frequency for triangular bins.", &high_frequency);
		opts->Register(prefix, "vtln-low-frequency", "Lower inflection point for the warping function.", &vtln_low_frequency);
		opts->Register(prefix, "vtln-high-frequency", "Higher inflection point of the warping function.", &vtln_high_frequency);
		opts->Register(prefix, "vtln-warping-factor", "VTLN warping factor.", &vtln_warping_factor);
	}

	MelFilterBank::MelFilterBank(const MelFilterBankOptions& options) {
		m_options = options;
		ValidateOptions();
		InitMelFilterBank();
	}

	void MelFilterBank::InitMelFilterBank() {
		// TODO: This might contain bugs and generally needs a proper rewrite, but I dont know enough about audio processing to do it
		field_x28.resize(m_options.num_bins, 0);
		field_x40.resize(m_options.num_bins);
		const auto fVar13 = logf(m_options.low_frequency / 700.0f + 1.0f);
		auto fVar14 = logf(m_options.high_frequency / 700.0f + 1.0f);
		fVar14 = (fVar14 * 1127.0 - fVar13 * 1127.0) / static_cast<float>(m_options.num_bins + 1);
		auto fVar17 = static_cast<float>(m_options.sample_rate) / static_cast<float>(m_options.num_fft_points);
		for (size_t b = 0; b < m_options.num_bins; b++) {
			auto local_60 = static_cast<float>(b) * fVar14 + fVar13 * 1127.0f;
			auto local_68 = local_60 + fVar14;
			auto local_5c = local_68 + fVar14;
			if (m_options.vtln_warping_factor != 1.0) {
				auto fVar15 = expf(local_60 / 1127.0);
				fVar15 = GetVtlnWarping((fVar15 - 1.0) * 700.0);
				local_60 = logf(fVar15 / 700.0 + 1.0);
				local_60 = local_60 * 1127.0;
				fVar15 = expf(local_68 / 1127.0);
				fVar15 = GetVtlnWarping((fVar15 - 1.0) * 700.0);
				local_68 = logf(fVar15 / 700.0 + 1.0);
				local_68 = local_68 * 1127.0;
				fVar15 = expf(local_5c / 1127.0);
				fVar15 = GetVtlnWarping((fVar15 - 1.0) * 700.0);
				local_5c = logf(fVar15 / 700.0 + 1.0);
				local_5c = local_5c * 1127.0;
			}
			auto fVar15 = expf(local_60 / 1127.0f);
			fVar15 = floorf(((fVar15 - 1.0f) * 700.0f) / fVar17);
			auto fVar16 = expf(local_5c / 1127.0f);
			fVar16 = ceilf(((fVar16 - 1.0f) * 700.0f) / fVar17);
			int iVar12 = (fVar15 + 1.0f > -1) ? (fVar15 + 1.0f) : 0;
			field_x28[b] = iVar12;
			int iVar4 = (fVar16 - 1.0f > m_options.num_fft_points / 2 - 1) ? (m_options.num_fft_points / 2 - 1) : (fVar16 - 1.0f);
			field_x40[b].Resize((iVar4 - iVar12) + 1);
			int iVar9 = 0;
			while (iVar4 >= iVar12) {
				auto idx = iVar9;
				do {
					fVar15 = logf(((float)iVar12 * fVar17) / 700.0 + 1.0);
					fVar15 = fVar15 * 1127.0;
					idx = iVar9;
					if (fVar15 <= local_68) break;
					iVar12 += 1;
					iVar9 += 1;
					field_x40[b][idx] = (local_5c - fVar15) / (local_5c - local_68);
				} while (iVar12 <= iVar4);
				if (iVar12 > iVar4) break;
				iVar12 += 1;
				iVar9 += 1;
				field_x40[b][idx] = (fVar15 - local_60) / (local_68 - local_60);
			}
		}
	}

	float MelFilterBank::GetVtlnWarping(float param_1) const {
		auto fVar1 = 1.0f / m_options.vtln_warping_factor;
		float fVar3 = m_options.vtln_low_frequency / std::max(fVar1, 1.0f);
		if (fVar3 <= param_1) {
			auto fVar2 = m_options.vtln_high_frequency / std::max(1.0f, fVar1);
			if (fVar2 <= param_1) {
				fVar3 = m_options.high_frequency;
				return fVar3 - ((fVar3 - fVar1 * fVar2) / (fVar3 - fVar2)) * (fVar3 - param_1);
			}
			return param_1 * fVar1;
		} else {
			return (param_1 - m_options.low_frequency) * ((fVar1 * fVar3 - m_options.low_frequency) / (fVar3 - m_options.low_frequency)) + m_options.low_frequency;
		}
	}

	void MelFilterBank::ComputeMelFilterBankEnergy(const VectorBase& input, const VectorBase& output) const {
		SNOWBOY_ASSERT(m_options.num_bins == param_2.size());
		for (size_t b = 0; b < output.size(); b++) {
			output[b] = field_x40[b].DotVec(input.Range(field_x28[b], field_x40[b].size()));
		}
	}

	void MelFilterBank::ValidateOptions() const {
		return;
	}

	void ComputeDctMatrixTypeIII(const MatrixBase& mat) {
		if (mat.empty()) return;
		auto fVar12 = sqrtf(1.0 / (float)mat.rows());
		for (size_t c = 0; c < mat.cols(); c++)
			mat(0, c) = fVar12;
		const auto pi_div_rows = M_PI / mat.rows();
		for (size_t r = 1; r < mat.rows(); r++) {
			auto sq = sqrtf(2.0f / mat.rows());
			for (size_t c = 0; c < mat.cols(); c++) {
				mat(r, c) = cosf((c + 0.5) * pi_div_rows * r) * sq;
			}
		}
	}

	void ComputeCepstralLifterCoeffs(float param_1, Vector* param_2) {
		for (size_t i = 0; i < param_2->size(); i++) {
			auto d = sin((static_cast<double>(i) * M_PI) / static_cast<double>(param_1));
			(*param_2)[i] = (d * (param_1 * 0.5) + 1.0);
		}
	}

	void ComputePowerSpectrumReal(const VectorBase& in, const VectorBase& out) {
		if (in.empty()) return;
		SNOWBOY_ASSERT(out.size() >= in.size() / 2);
		out[0] = in[0] * in[0];
		for (size_t i = 0; i < in.size() / 2 - 1; i++) {
			out[i + 1] = in[i * 2 + 3] * in[i * 2 + 3] + in[i * 2 + 2] * in[i * 2 + 2];
		}
		//data.Resize(data.size() / 2, MatrixResizeType::kCopyData);
	}

	FftItf::~FftItf() {}

	Fft::Fft(const FftOptions& options) {
		m_options = options;
		Init();
	}

	void Fft::DoFft(bool inverse, Vector* data) const noexcept {
		SNOWBOY_ASSERT(!data->HasNan());
		if (m_options.field_x00) {
			if (m_options.num_fft_points == 1) return;
			if (inverse) {
				DoProcessingForReal(inverse, data);
				DoBitReversalSorting(m_bit_reversal_index, *data);
				DoDanielsonLanczos(true, *data);
				SNOWBOY_ASSERT(!data->HasNan() && !data->HasInfinity());
				return;
			}
		}
		DoBitReversalSorting(m_bit_reversal_index, *data);
		SNOWBOY_ASSERT(!data->HasNan() && !data->HasInfinity());
		DoDanielsonLanczos(inverse, *data);
		SNOWBOY_ASSERT(!data->HasNan() && !data->HasInfinity());
		if (m_options.field_x00 <= inverse) return;
		DoProcessingForReal(inverse, data);
	}

	void Fft::DoDanielsonLanczos(bool inverse, const VectorBase& data) const noexcept {
		const auto num_bits = GetNumBits(data.size() / 2);
		for (size_t bit_idx = 1; bit_idx <= num_bits; bit_idx += 1) {
			const auto bit_value = 1 << (bit_idx & 0x1f);
			for (size_t local_68 = 0; local_68 < data.size() / 2; local_68 += bit_value) {
				long lVar14 = local_68 * 2 + 1 + bit_value;
				long lVar15 = (local_68 * 2);
				for (auto iVar11 = 0; iVar11 != bit_value / 2; iVar11++) {
					auto twiddle = GetTwiddleFactor(bit_value, iVar11);
					if (inverse) twiddle.second *= -1;
					auto fVar16 = data[lVar15 + bit_value];
					auto fVar18 = fVar16 * twiddle.first - twiddle.second * data[lVar14];
					fVar16 = data[lVar14] * twiddle.first + fVar16 * twiddle.second;
					data[lVar15 + bit_value] = data[lVar15] - fVar18;
					data[lVar14] = data[lVar15 + 1] - fVar16;
					data[lVar15] += fVar18;
					data[lVar15 + 1] += fVar16;
					lVar14 += 2;
					lVar15 += 2;
				}
			}
		}
		if (inverse) {
			const auto f = 1.0f / static_cast<float>(data.size() / 2);
			for (size_t i = 0; i < data.size(); i++)
				data[i] = data[i] * f;
		}
	}

	void Fft::DoBitReversalSorting(const std::vector<unsigned int>& reversal_index, const VectorBase& data) noexcept {
		for (size_t i = 0; i < data.size(); i++) {
			auto idx = reversal_index[i % reversal_index.size()];
			if (i < idx) {
				std::swap(data[i], data[idx]);
			}
		}
	}

	void Fft::ComputeTwiddleFactor(size_t len) {
		m_twiddle_factors.resize(len);
		auto ptr = m_twiddle_factors.data();
		ptr[0] = 1.0;
		ptr[1] = 0;

		float _sin, _cos;
		sincosf((-M_PI * 2) / len, &_sin, &_cos);
		float tempa = 0;
		float tempb = 1;
		for (size_t i = 2; i < len - 4; i += 4) {
			ptr[i] = _cos * tempb - tempa * _sin;
			ptr[i + 1] = tempa = tempb * _sin + tempa * _cos;
			ptr[i + 2] = tempb = _cos * ptr[i] - _sin * tempa;
			ptr[i + 3] = tempa = ptr[i] * _sin + tempa * _cos;
		}
		ptr[len - 2] = _cos * tempb - tempa * _sin;
		ptr[len - 1] = tempa = tempb * _sin + tempa * _cos;
	}

	void Fft::ComputeBitReversalIndex(size_t len) {
		if (len == 0) {
			m_bit_reversal_index.clear();
			return;
		}
		m_bit_reversal_index.resize(len * 2, -1);
		auto num_bits = GetNumBits(len);
		for (size_t i = 0; i < len; i++) {
			auto x = ReverseBit(i, num_bits) * 2;
			m_bit_reversal_index[i * 2] = x;
			m_bit_reversal_index[i * 2 + 1] = x + 1;
		}
	}

	void Fft::DoProcessingForReal(bool inverse, Vector* param_2) const noexcept {
		// TODO: Could someone with any clue about maths explain to me what the fuck happens here ?
		// I reversed it and it seems to return the correct values, but no clue about why it does....
		const auto num_pts = m_options.num_fft_points;
		const auto ptr = param_2->data();
		auto f = ptr[0];
		int iVar11 = (-1 < num_pts) ? num_pts : (num_pts + 3);
		ptr[0] = f + ptr[1];
		ptr[1] = f - ptr[1];

		for (auto iVar13 = 1; iVar13 <= iVar11 / 4; iVar13 += 1) {
			const auto lVar9 = num_pts - (iVar13 - 1) * 2;
			const auto twiddle = GetTwiddleFactor(num_pts, inverse ? (static_cast<float>(num_pts) * 0.5f - iVar13) : iVar13);
			const auto fVar4 = ptr[lVar9 - 1];
			const auto fVar5 = ptr[lVar9 - 2];
			const auto fVar6 = ptr[iVar13 * 2];
			const auto fVar7 = ptr[iVar13 * 2 + 1];
			ptr[iVar13 * 2] = (twiddle.first * fVar7 + (twiddle.second + 1.0f) * fVar6 + (1.0f - twiddle.second) * fVar5 + fVar4 * twiddle.first) * 0.5f;
			ptr[iVar13 * 2 + 1] = ((twiddle.second + 1.0f) * fVar7 + ((fVar5 * twiddle.first - (1.0f - twiddle.second) * fVar4) - twiddle.first * fVar6)) * 0.5f;
			if (iVar13 * 2 != lVar9 - 2) {
				ptr[lVar9 - 2] = ((((twiddle.second + 1.0f) * fVar5 - fVar4 * twiddle.first) + (1.0f - twiddle.second) * fVar6) - twiddle.first * fVar7) * 0.5f;
				ptr[lVar9 - 1] = (((fVar5 * twiddle.first + fVar4 * (twiddle.second + 1.0f)) - fVar6 * twiddle.first) - fVar7 * (1.0f - twiddle.second)) * 0.5f;
			}
		}
		if (inverse) {
			*ptr *= 0.5f;
			ptr[1] *= 0.5f;
		}
	}

	size_t Fft::GetNumBits(size_t param_1) noexcept {
		return param_1 == 0 ? 0 : (31 - __builtin_clz(param_1));
	}

	std::pair<float, float> Fft::GetTwiddleFactor(int param_1, int param_2) const noexcept {
		auto size = m_twiddle_factors.size();
		auto idx = (size / param_1) * param_2 * 2;
		if (idx < size) {
			return {m_twiddle_factors[idx], m_twiddle_factors[idx + 1]};
		} else {
			return {m_twiddle_factors[size - idx] * -1.0f, m_twiddle_factors[(size - idx) + 1] * -1.0f};
		}
	}

	void Fft::Init() {
		auto reversal_idx_len = m_options.num_fft_points;
		if (m_options.field_x00) reversal_idx_len /= 2;
		ComputeBitReversalIndex(reversal_idx_len);
		ComputeTwiddleFactor(m_options.num_fft_points);
	}

	size_t Fft::ReverseBit(size_t val, size_t num_bits) noexcept {
		if (val == 0) return 0;
		unsigned int res = 0;
		while (val != 0) {
			num_bits -= 1;
			res = (res * 2) | (val & 1);
			val >>= 1;
		}
		return res << (num_bits & 0x1f);
	}

	void Fft::SetOptions(const FftOptions& opts) {
		m_options = opts;
		Init();
	}

	void Fft::DoFft(Vector* data) const noexcept {
		DoFft(false, data);
	}

	void Fft::DoIfft(Vector* data) const noexcept {
		DoFft(true, data);
	}

	Fft::~Fft() {}

} // namespace snowboy
