#include <audio-lib.h>
#include <matrix-wrapper.h>
#include <snowboy-debug.h>
#include <types.h>

namespace snowboy {
	float GetMaxWaveAmplitude(const WaveHeader& hdr) {
#ifndef NDEBUG
		if (hdr.wBitsPerSample != 8 && hdr.wBitsPerSample != 16 && hdr.wBitsPerSample != 32)
			SNOWBOY_ERROR() << "Undefined bits_per_sample: " << hdr.wBitsPerSample << ". Expecting 8,16 or 32.";
#endif
		return (1 << hdr.wBitsPerSample) - 1;
	}

	float GetMaxWaveAmplitude(int nbits) {
#ifndef NDEBUG
		if (nbits != 8 && nbits != 16 && nbits != 32)
			SNOWBOY_ERROR() << "Undefined bits_per_sample: " << nbits << ". Expecting 8,16 or 32.";
#endif
		return (1 << nbits) - 1;
	}

	void ReadRawWaveFromString(const WaveHeader& hdr, const std::string& data, Matrix* data_out) {
		// Note: Not 100% sure about this stuff...
		data_out->Resize(hdr.wChannels, data.size() / hdr.wBlockAlign);
		if (hdr.wBitsPerSample == 8) {
			auto dptr = reinterpret_cast<uint8_t*>(const_cast<char*>(data.data()));
			for (size_t c = 0; c < data_out->m_cols; c++) {
				for (size_t r = 0; r < data_out->m_rows; r++) {
					data_out->m_data[r * data_out->m_stride + c] = *dptr;
					dptr++;
				}
			}
		} else if (hdr.wBitsPerSample == 16) {
			auto dptr = reinterpret_cast<uint16_t*>(const_cast<char*>(data.data()));
			for (size_t c = 0; c < data_out->m_cols; c++) {
				for (size_t r = 0; r < data_out->m_rows; r++) {
					data_out->m_data[r * data_out->m_stride + c] = *dptr;
					dptr++;
				}
			}
		} else if (hdr.wBitsPerSample == 32) {
			auto dptr = reinterpret_cast<uint32_t*>(const_cast<char*>(data.data()));
			for (size_t c = 0; c < data_out->m_cols; c++) {
				for (size_t r = 0; r < data_out->m_rows; r++) {
					data_out->m_data[r * data_out->m_stride + c] = *dptr;
					dptr++;
				}
			}
		} else {
			SNOWBOY_ERROR() << "Undefined bits_per_sample: " << hdr.wBitsPerSample << ". Expecting 8,16 or 32.";
		}
	}

	void WriteRawWaveToString(const WaveHeader& hdr, const MatrixBase& data, std::string* data_out) {
		// Note: Not 100% sure about this stuff...
		data_out->resize(hdr.wChannels * data.m_cols * (hdr.wBitsPerSample >> 3));
		if (hdr.wBitsPerSample == 8) {
			auto dptr = reinterpret_cast<uint8_t*>(const_cast<char*>(data_out->data()));
			for (size_t c = 0; c < data.m_cols; c++) {
				for (size_t r = 0; r < data.m_rows; r++) {
					*dptr = data.m_data[r * data.m_stride + c];
					dptr++;
				}
			}
		} else if (hdr.wBitsPerSample == 16) {
			auto dptr = reinterpret_cast<uint16_t*>(const_cast<char*>(data_out->data()));
			for (size_t c = 0; c < data.m_cols; c++) {
				for (size_t r = 0; r < data.m_rows; r++) {
					*dptr = data.m_data[r * data.m_stride + c];
					dptr++;
				}
			}
		} else if (hdr.wBitsPerSample == 32) {
			auto dptr = reinterpret_cast<uint32_t*>(const_cast<char*>(data_out->data()));
			for (size_t c = 0; c < data.m_cols; c++) {
				for (size_t r = 0; r < data.m_rows; r++) {
					*dptr = data.m_data[r * data.m_stride + c];
					dptr++;
				}
			}
		} else {
			SNOWBOY_ERROR() << "Undefined bits_per_sample: " << hdr.wBitsPerSample << ". Expecting 8,16 or 32.";
		}
	}
} // namespace snowboy