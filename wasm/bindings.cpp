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

static int SnowboyDetect_RunDetectionI16(snowboy::SnowboyDetect& self, long jsHeapAddr, int len, bool is_end = false) {
	const int16_t* data = (const int16_t*)jsHeapAddr;
	return self.SnowboyDetect::RunDetection(data, len, false);
}

static int SnowboyDetect_RunDetectionI32(snowboy::SnowboyDetect& self, long jsHeapAddr, int len, bool is_end = false) {
	const int32_t* data = (const int32_t*)jsHeapAddr;
	return self.SnowboyDetect::RunDetection(data, len, false);
}

static int SnowboyDetect_RunDetectionF32(snowboy::SnowboyDetect& self, long jsHeapAddr, int len, bool is_end = false) {
	const float* data = (const float*)jsHeapAddr;
	return self.SnowboyDetect::RunDetection(data, len, false);
}

static snowboy::SnowboyVad* makeVad(const std::string& res_filename) {
	try {
		return new snowboy::SnowboyVad(res_filename.c_str());
	} catch (std::exception& e) {
		std::printf("Exception in SnowboyVad ctor: %s\n", e.what());
		throw;
	}
}

static int SnowboyVad_RunVadI16(snowboy::SnowboyVad& self, long jsHeapAddr, int len, bool is_end = false) {
	const int16_t* data = (const int16_t*)jsHeapAddr;
	return self.SnowboyVad::RunVad(data, len, false);
}

static int SnowboyVad_RunVadI32(snowboy::SnowboyVad& self, long jsHeapAddr, int len, bool is_end = false) {
	const int32_t* data = (const int32_t*)jsHeapAddr;
	return self.SnowboyVad::RunVad(data, len, false);
}

static int SnowboyVad_RunVadF32(snowboy::SnowboyVad& self, long jsHeapAddr, int len, bool is_end = false) {
	const float* data = (const float*)jsHeapAddr;
	return self.SnowboyVad::RunVad(data, len, false);
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
		.function("RunDetectionI16", &SnowboyDetect_RunDetectionI16)
		.function("RunDetectionI32", &SnowboyDetect_RunDetectionI32)
		.function("RunDetectionF32", &SnowboyDetect_RunDetectionF32);

	class_<snowboy::SnowboyVad>("SnowboyVad")
		.constructor(&makeVad, allow_raw_pointers())
		.function("SampleRate", &snowboy::SnowboyVad::SampleRate)
		.function("NumChannels", &snowboy::SnowboyVad::NumChannels)
		.function("BitsPerSample", &snowboy::SnowboyVad::BitsPerSample)
		.function("SetAudioGain", &snowboy::SnowboyVad::SetAudioGain)
		.function("ApplyFrontend", &snowboy::SnowboyVad::ApplyFrontend)
		.function("RunVadI16", &SnowboyVad_RunVadI16)
		.function("RunVadI32", &SnowboyVad_RunVadI32)
		.function("RunVadF32", &SnowboyVad_RunVadF32);
}
