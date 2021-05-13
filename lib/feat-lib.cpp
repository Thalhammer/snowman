#include <feat-lib.h>
#include <math.h>
#include <matrix-wrapper.h>
#include <snowboy-debug.h>
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

	MelFilterBank::~MelFilterBank() {}

	void MelFilterBank::InitMelFilterBank() {
		// TODO: This might contain bugs and generally needs a proper rewrite, but I dont know enough about audio processing to do it
		field_x28.resize(m_options.num_bins, 0);
		field_x40.resize(m_options.num_bins);
		auto fVar13 = logf(m_options.low_frequency / 700.0f + 1.0f);
		auto fVar14 = logf(m_options.high_frequency / 700.0f + 1.0f);
		fVar14 = (fVar14 * 1127.0 - fVar13 * 1127.0) / static_cast<float>(m_options.num_bins + 1);
		auto fVar17 = static_cast<float>(m_options.sample_rate) / static_cast<float>(m_options.num_fft_points);
		for (int b = 0; b < m_options.num_bins; b++) {
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
					field_x40[b].m_data[idx] = (local_5c - fVar15) / (local_5c - local_68);
				} while (iVar12 <= iVar4);
				if (iVar12 > iVar4) break;
				iVar12 += 1;
				iVar9 += 1;
				field_x40[b].m_data[idx] = (fVar15 - local_60) / (local_68 - local_60);
			}
		}
	}

	float MelFilterBank::GetVtlnWarping(float param_1) const {
		auto fVar1 = 1.0f / m_options.vtln_warping_factor;
		float fVar3 = m_options.vtln_low_frequency / ((fVar1 < 1.0) ? fVar1 : 1.0);
		auto fVar2 = fVar1;
		if (fVar1 <= 1.0f) {
			fVar2 = 1.0f;
		}
		if (fVar3 <= param_1) {
			fVar2 = m_options.vtln_high_frequency / fVar2;
			if (fVar2 <= param_1) {
				fVar3 = m_options.high_frequency;
				return fVar3 - ((fVar3 - fVar1 * fVar2) / (fVar3 - fVar2)) * (fVar3 - param_1);
			}
			return param_1 * fVar1;
		} else {
			return (param_1 - m_options.low_frequency) * ((fVar1 * fVar3 - m_options.low_frequency) / (fVar3 - m_options.low_frequency)) + m_options.low_frequency;
		}
	}

	void MelFilterBank::ComputeMelFilterBankEnergy(const VectorBase& param_1, Vector* param_2) const {
		if (m_options.num_bins != param_2->m_size) param_2->Resize(m_options.num_bins);
		for (int b = 0; b < param_2->m_size; b++) {
			auto f = field_x40[b].DotVec(param_1.Range(field_x28[b], field_x40[b].m_size));
			param_2->m_data[b] = f;
		}
	}

	void MelFilterBank::ValidateOptions() const {
		return;
	}

	void ComputeDctMatrixTypeIII(Matrix* mat) {
		if (mat->m_rows == 0 || mat->m_cols == 0) return;
		auto fVar12 = sqrtf(1.0 / (float)mat->m_rows);
		for (size_t c = 0; c < mat->m_cols; c++)
			mat->m_data[c] = fVar12;
		for (size_t r = 1; r < mat->m_rows; r++) {
			auto sq = sqrtf(2.0 / mat->m_rows);
			for (size_t c = 0; c < mat->m_cols; c++) {
				// TODO: In the name of performance we might wanna use cosf here.
				// It does cause minor value differences but it should not impact anything
				mat->m_data[r * mat->m_stride + c] = cos((c + 0.5) * (M_PI / mat->m_rows) * r) * sq;
			}
		}
	}

	void ComputeCepstralLifterCoeffs(float param_1, Vector* param_2) {
		for (int i = 0; i < param_2->m_size; i++) {
			auto d = sin((static_cast<double>(i) * M_PI) / static_cast<double>(param_1));
			param_2->m_data[i] = (d * (param_1 * 0.5) + 1.0);
		}
	}

	void ComputePowerSpectrumReal(Vector* data) {
		if (data->m_size == 0) return;
		auto ptr = data->m_data;
		float f = ptr[0] * ptr[0];
		for (int i = 0; i < data->m_size / 2 - 1; i++) {
			ptr[i + 1] = ptr[i * 2 + 3] * ptr[i * 2 + 3] + ptr[i * 2 + 2] * ptr[i * 2 + 2];
		}
		ptr[0] = f;
		data->Resize(data->m_size / 2, MatrixResizeType::kCopyData);
	}

	FftItf::~FftItf() {}

	Fft::Fft(const FftOptions& options) {
		m_options = options;
		Init();
	}

	void Fft::DoFft(bool inverse, Vector* data) const {
		if (m_options.field_x00) {
			if (m_options.num_fft_points == 1) return;
			if (inverse) {
				DoProcessingForReal(true, data);
				DoBitReversalSorting(m_bit_reversal_index, data);
				DoDanielsonLanczos(true, data);
				return;
			}
		}
		DoBitReversalSorting(m_bit_reversal_index, data);
		DoDanielsonLanczos(inverse, data);
		if (m_options.field_x00 <= inverse) return;
		DoProcessingForReal(inverse, data);
	}

	void Fft::DoDanielsonLanczos(bool inverse, Vector* data) const {
		const auto uVar6 = data->m_size / 2;
		const auto pfVar5 = data->m_data;
		const auto iVar7 = snowboy::Fft::GetNumBits(uVar6);
		for (auto local_54 = 1; local_54 <= iVar7; local_54 += 1) {
			const auto iVar9 = 1 << (local_54 & 0x1f);
			for (auto local_68 = 0; local_68 < (int)uVar6; local_68 += iVar9) {
				auto lVar14 = (long)local_68 * 2 + 1 + iVar9;
				auto lVar15 = (long)(local_68 * 2);
				for (auto iVar11 = 0; iVar11 != iVar9 / 2; iVar11++) {
					float local_40, local_3c;
					snowboy::Fft::GetTwiddleFactor(iVar9, iVar11, &local_40, &local_3c);
					if (inverse) local_3c *= -1;
					auto fVar16 = pfVar5[lVar15 + iVar9];
					auto fVar18 = fVar16 * local_40 - local_3c * pfVar5[lVar14];
					fVar16 = pfVar5[lVar14] * local_40 + fVar16 * local_3c;
					pfVar5[lVar15 + iVar9] = pfVar5[lVar15] - fVar18;
					pfVar5[lVar14] = pfVar5[lVar15 + 1] - fVar16;
					pfVar5[lVar15] += fVar18;
					pfVar5[lVar15 + 1] += fVar16;
					lVar14 += 2;
					lVar15 += 2;
				}
			}
		}
		if (inverse) {
			for (int i = 0; i < data->m_size; i++)
				data->m_data[i] = data->m_data[i] / static_cast<float>(uVar6);
		}
	}

	void Fft::DoBitReversalSorting(const std::vector<int>& reversal_index, Vector* data) const {
		for (int i = 0; i < data->m_size; i++) {
			if (i < reversal_index[i]) {
				auto x = data->m_data[i];
				data->m_data[i] = data->m_data[reversal_index[i]];
				data->m_data[reversal_index[i]] = x;
			}
		}
	}

	void Fft::ComputeTwiddleFactor(int len) {
		m_twiddle_factors.resize(len);
		auto ptr = m_twiddle_factors.data();
		ptr[0] = 1.0;
		ptr[1] = 0;

		float _sin, _cos;
		sincosf((-M_PI * 2) / len, &_sin, &_cos);
		float tempa = 0;
		float tempb = 1;
		for (int i = 2; i < len - 4; i += 4) {
			float fvar13 = _cos * tempb - tempa * _sin;
			ptr[i] = fvar13;
			ptr[i + 1] = tempa = tempb * _sin + tempa * _cos;
			ptr[i + 2] = tempb = _cos * fvar13 - _sin * tempa;
			ptr[i + 3] = tempa = fvar13 * _sin + tempa * _cos;
		}
		ptr[len - 2] = _cos * tempb - tempa * _sin;
		ptr[len - 1] = tempa = tempb * _sin + tempa * _cos;
	}

	void Fft::ComputeBitReversalIndex(int len, std::vector<int>* index) const {
		if (len == 0) {
			index->clear();
			return;
		}
		index->resize(len * 2, -1);
		auto num_bits = GetNumBits(len);
		auto ptr = index->data();
		for (int i = 0; i < len; i++) {
			auto x = ReverseBit(i, num_bits) * 2;
			*ptr = x;
			ptr++;
			*ptr = x + 1;
			ptr++;
		}
	}

	void Fft::DoProcessingForReal(bool param_1, Vector* param_2) const {
		// TODO: Could someone with any clue about maths explain to me what the fuck happens here ?
		// I reversed it and it seems to return the correct values, but no clue about why it does....
		const auto num_pts = m_options.num_fft_points;
		const auto ptr = param_2->m_data;
		auto f = ptr[0];
		int iVar11 = (-1 < num_pts) ? num_pts : (num_pts + 3);
		ptr[0] = f + ptr[1];
		ptr[1] = f - ptr[1];
		if (0 < iVar11 / 4) {
			auto lVar12 = 2;
			auto lVar9 = num_pts;
			for (auto iVar13 = 1; iVar13 <= iVar11 / 4; iVar13 += 1) {
				float twiddle_a, twiddle_b;
				snowboy::Fft::GetTwiddleFactor(num_pts, param_1 ? (static_cast<double>(num_pts) * 0.5 - iVar13) : iVar13, &twiddle_a, &twiddle_b);
				const auto fVar4 = ptr[lVar9 - 1];
				const auto fVar5 = ptr[lVar9 - 2];
				const auto fVar6 = ptr[lVar12];
				const auto fVar7 = ptr[lVar12 + 1];
				ptr[lVar12] = (twiddle_a * fVar7 + (twiddle_b + 1.0) * fVar6 + (1.0 - twiddle_b) * fVar5 + fVar4 * twiddle_a) * 0.5;
				ptr[lVar12 + 1] = ((twiddle_b + 1.0) * fVar7 + ((fVar5 * twiddle_a - (1.0 - twiddle_b) * fVar4) - twiddle_a * fVar6)) * 0.5;
				if (iVar13 * 2 != lVar9 - 2) {
					ptr[lVar9 - 2] = ((((twiddle_b + 1.0) * fVar5 - fVar4 * twiddle_a) + (1.0 - twiddle_b) * fVar6) - twiddle_a * fVar7) * 0.5;
					ptr[lVar9 - 1] = (((fVar5 * twiddle_a + fVar4 * (twiddle_b + 1.0)) - fVar6 * twiddle_a) - fVar7 * (1.0 - twiddle_b)) * 0.5;
				}
				lVar12 += 2;
				lVar9 -= 2;
			}
		}
		if (param_1 != false) {
			*ptr = *ptr * 0.5;
			ptr[1] = ptr[1] * 0.5;
		}
	}

	unsigned int Fft::GetNumBits(unsigned int param_1) const {
		// Note: This is the original function, but using __builtin_clz should generate way faster code
		//unsigned int res = 0;
		//while(param_1 > 1) {
		//    res += 1;
		//    param_1 >>= 1;
		//}
		//return res;
		// TODO: Use C++20 bit operations once we build for real
		return param_1 == 0 ? 0 : (31 - __builtin_clz(param_1));
	}

	void Fft::GetTwiddleFactor(int param_1, int param_2, float* param_3, float* param_4) const {
		auto size = m_twiddle_factors.size();
		auto idx = (size / param_1) * param_2 * 2;
		if (idx < size) {
			*param_3 = m_twiddle_factors[idx];
			*param_4 = m_twiddle_factors[idx + 1];
		} else {
			*param_3 = m_twiddle_factors[size - idx] * -1;
			*param_3 = m_twiddle_factors[(size - idx) + 1] * -1;
		}
	}

	void Fft::Init() {
		if (!m_options.field_x00)
			field_x10 = m_options.num_fft_points;
		else
			field_x10 = m_options.num_fft_points / 2;
		ComputeBitReversalIndex(field_x10, &m_bit_reversal_index);
		ComputeTwiddleFactor(m_options.num_fft_points);
	}

	unsigned int Fft::ReverseBit(unsigned int a, unsigned int b) const {
		// TODO: Can we optimize this ?
		if (a == 0) return 0;
		unsigned int res = 0;
		while (a != 0) {
			b -= 1;
			res = (res * 2) | (a & 1);
			a >>= 1;
		}
		return res << (b & 0x1f);
	}

	void Fft::SetOptions(const FftOptions& opts) {
		m_options = opts;
		Init();
	}

	void Fft::DoFft(Vector* data) const {
		DoFft(false, data);
	}

	void Fft::DoIfft(Vector* data) const {
		DoFft(true, data);
	}

	Fft::~Fft() {}

} // namespace snowboy
