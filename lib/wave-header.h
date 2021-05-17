#pragma once

namespace snowboy {

	struct WaveHeader {
		char chunkID[4] = {'R', 'I', 'F', 'F'};
		uint32_t chunkSize = 0;
		char riffType[4] = {'W', 'A', 'V', 'E'};
		char fmtHeader[4] = {'f', 'm', 't', ' '};
		uint32_t fmtChunkSize = 16;
		uint16_t wFormatTag = 1;
		uint16_t wChannels = 1;
		uint32_t dwSamplesPerSec = 16000;
		uint32_t dwAvgBytesPerSec = 32000;
		uint16_t wBlockAlign = 2;
		uint16_t wBitsPerSample = 16;
		char dataHeader[4] = {'d', 'a', 't', 'a'};
		uint32_t dataChunkSize = 0;
	};
} // namespace snowboy