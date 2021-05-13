#pragma once
#include <string>

namespace snowboy {
	struct WaveHeader;
	struct Matrix;
	struct MatrixBase;
	float GetMaxWaveAmplitude(const WaveHeader& hdr);
	float GetMaxWaveAmplitude(int nbits);
	void ReadRawWaveFromString(const WaveHeader& hdr, const std::string& data, Matrix* data_out);
	void WriteRawWaveToString(const WaveHeader& hdr, const MatrixBase& data, std::string* data_out);
} // namespace snowboy