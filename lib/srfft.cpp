#include <cmath>
#include <cstring>
#include <srfft.h>

namespace snowboy {
	SplitRadixFft::SplitRadixFft(const FftOptions& options)
		: m_options{options} {
		Init();
	}

	void SplitRadixFft::DoFft(bool inverse, Vector* data) const {
		if (m_options.field_x00) {
			if (m_options.num_fft_points == 1) return;
			if (inverse) {
				DoProcessingForReal(inverse, data);
				DoComplexFft(inverse, data);
				return;
			}
		}
		DoComplexFft(inverse, data);
		if (m_options.field_x00 <= inverse) return;
		DoProcessingForReal(inverse, data);
	}

	void SplitRadixFft::DoComplexFftRecursive(int, float*, float*) const {
		// TODO
	}

	void SplitRadixFft::DoComplexFftComputation(bool inverse, float* param_2, float* param_3) const {
		if (!inverse) {
			DoComplexFftRecursive(field_x14, param_2, param_3);
			if (field_x14 > 1) {
				BitReversePermute(field_x14, param_2);
				BitReversePermute(field_x14, param_3);
			}
		} else {
			DoComplexFftRecursive(field_x14, param_3, param_2);
			if (field_x14 > 1) {
				BitReversePermute(field_x14, param_3);
				BitReversePermute(field_x14, param_2);
			}
			for (size_t i = 0; i < field_x10; i++) {
				param_3[i] = param_3[i] / static_cast<float>(field_x10);
				param_2[i] = param_2[i] / static_cast<float>(field_x10);
			}
		}
	}

	void SplitRadixFft::DoComplexFft(bool inverse, Vector* data) const {
		std::vector<float> temp;
		temp.resize(field_x10);
		auto ptr = data->data();
		for (size_t i = 0; i < field_x10; i++) {
			ptr[i] = ptr[i * 2];
			temp[i] = ptr[i * 2 + 1];
		}
		memcpy(ptr + field_x10, temp.data(), field_x10 * sizeof(float));
		DoComplexFftComputation(inverse, data->data(), data->data() + field_x10);
		memcpy(temp.data(), ptr + field_x10, field_x10 * sizeof(float));
		auto uVar9 = field_x10 - 1;
		if (-1 < (int)uVar9) {
			auto lVar8 = field_x10 * sizeof(float) - 4;
			unsigned long lVar3 = 0;
			auto pfVar6 = ptr + (int)(uVar9 * 2);
			do {
				*pfVar6 = ptr[(lVar3 + lVar8) / 4];
				lVar3 += -4;
				pfVar6[1] = temp[(lVar3 + lVar8) / 4];
				pfVar6 = pfVar6 + -2;
			} while (lVar3 != ~(unsigned long)uVar9 * 4);
		}
	}

	void SplitRadixFft::ComputeTables() {
		// TODO
	}

	void SplitRadixFft::BitReversePermute(int param_1, float* param_2) const {
		auto iVar9 = 1 << ((uint8_t)(param_1 >> 1) & 0x1f);
		for (int a = 1; a < iVar9; a++) {
			const auto index = field_x18[a];
			std::swap(param_2[iVar9 * index], param_2[a]);
			for (int i = 1; i != index; i++) {
				std::swap(param_2[field_x18[i] + iVar9 * index], param_2[i * iVar9 + a]);
			}
		}
	}

	void SplitRadixFft::DoProcessingForReal(bool, Vector*) const {
		// TODO
	}

	void SplitRadixFft::Init() {
		field_x10 = m_options.num_fft_points;
		if (m_options.field_x00) field_x10 /= 2;
		auto temp = floorf(logf(field_x10) / 0.6931471805599453 + 0.5);
		field_x14 = temp;
		temp = powf(2.0, temp);
		if (field_x10 < temp) field_x14--;
		ComputeTables();
	}

	void SplitRadixFft::SetOptions(const FftOptions& options) {
		m_options = options;
		Init();
	}

	void SplitRadixFft::DoFft(Vector* data) const noexcept {
		DoFft(false, data);
	}

	void SplitRadixFft::DoIfft(Vector* data) const noexcept {
		DoFft(true, data);
	}

	SplitRadixFft::~SplitRadixFft() {}

} // namespace snowboy
