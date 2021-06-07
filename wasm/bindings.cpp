// Copyright 2021 Ciaran O'Reilly

#include "../lib/snowboy-detect.h"
#include <emscripten/bind.h>

using namespace emscripten;

static snowboy::SnowboyDetect* makeDetector(const std::string& resource_filename, const std::string& model_str) {
	try {
		return new snowboy::SnowboyDetect(resource_filename.c_str(), model_str.c_str());
	} catch (std::exception& e) {
		printf("Exception in SnowboyDetect ctor: %s\n", e.what());
		throw;
	}
}

static int SnowboyDetect_RunDetection(snowboy::SnowboyDetect& self, long jsHeapAddr, int len, bool is_end = false) {
	const int16_t* fdata = (const int16_t*)jsHeapAddr;
	//std::printf("RunDetection received len=%d 0=%d %d=%d\n", len, fdata[0], len - 1, fdata[len - 1]);

	return self.SnowboyDetect::RunDetection(fdata, len, false);
}

static snowboy::SnowboyVad* makeVad(const std::string& res_filename) {
	try {
		return new snowboy::SnowboyVad(res_filename.c_str());
	} catch (std::exception& e) {
		std::printf("Exception in SnowboyVad ctor: %s\n", e.what());
		throw;
	}
}

static int SnowboyVad_RunVad(snowboy::SnowboyVad& self, long jsHeapAddr, int len, bool is_end = false) {
	const int32_t* fdata = (const int32_t*)jsHeapAddr;
	std::printf("RunVad received len=%d 0=%d %d=%d\n", len, fdata[0], len - 1, fdata[len - 1]);

	return self.SnowboyVad::RunVad(fdata, len, false);
}

EMSCRIPTEN_BINDINGS(snowman) {
	class_<snowboy::SnowboyDetect>("SnowboyDetect")
		.constructor(&makeDetector, allow_raw_pointers())
		.function("SampleRate", &snowboy::SnowboyDetect::SampleRate)
		.function("NumChannels", &snowboy::SnowboyDetect::NumChannels)
		.function("BitsPerSample", &snowboy::SnowboyDetect::BitsPerSample)
		.function("GetSensitivity", &snowboy::SnowboyDetect::GetSensitivity)
		.function("NumHotwords", &snowboy::SnowboyDetect::NumHotwords)
		.function("SetAudioGain", &snowboy::SnowboyDetect::SetAudioGain)
		.function("ApplyFrontend", &snowboy::SnowboyDetect::ApplyFrontend)
		.function("Reset", &snowboy::SnowboyDetect::Reset)
		.function("RunDetection", &SnowboyDetect_RunDetection);

	class_<snowboy::SnowboyVad>("SnowboyVad")
		.constructor(&makeVad, allow_raw_pointers())
		.function("SampleRate", &snowboy::SnowboyVad::SampleRate)
		.function("NumChannels", &snowboy::SnowboyVad::NumChannels)
		.function("BitsPerSample", &snowboy::SnowboyVad::BitsPerSample)
		.function("SetAudioGain", &snowboy::SnowboyVad::SetAudioGain)
		.function("ApplyFrontend", &snowboy::SnowboyVad::ApplyFrontend)
		.function("RunVad", &SnowboyVad_RunVad);
}
