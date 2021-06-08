// Copyright 2021 Ciaran O'Reilly

#include "../lib/snowboy-detect.h"
#include <emscripten/bind.h>

using namespace emscripten;

static snowboy::SnowboyDetect* makeDetector(const std::string& resource_filename, const std::string& model_str) {
	try {
		return new snowboy::SnowboyDetect(resource_filename, model_str);
	} catch (std::exception& e) {
		printf("Exception in SnowboyDetect ctor: %s\n", e.what());
		throw;
	}
}

static int SnowboyDetect_RunDetectionI16(snowboy::SnowboyDetect& self, long jsHeapAddr, int len, bool is_end = false) {
	return self.RunDetection(reinterpret_cast<const int16_t*>(jsHeapAddr), len, false);
}

static int SnowboyDetect_RunDetectionI32(snowboy::SnowboyDetect& self, long jsHeapAddr, int len, bool is_end = false) {
	return self.RunDetection(reinterpret_cast<const int32_t*>(jsHeapAddr), len, false);
}

static int SnowboyDetect_RunDetectionF32(snowboy::SnowboyDetect& self, long jsHeapAddr, int len, bool is_end = false) {
	return self.RunDetection(reinterpret_cast<const float*>(jsHeapAddr), len, false);
}

static snowboy::SnowboyVad* makeVad(const std::string& res_filename) {
	try {
		return new snowboy::SnowboyVad(res_filename);
	} catch (std::exception& e) {
		std::printf("Exception in SnowboyVad ctor: %s\n", e.what());
		throw;
	}
}

static int SnowboyVad_RunVadI16(snowboy::SnowboyVad& self, long jsHeapAddr, int len, bool is_end = false) {
	return self.RunVad(reinterpret_cast<const int16_t*>(jsHeapAddr), len, false);
}

static int SnowboyVad_RunVadI32(snowboy::SnowboyVad& self, long jsHeapAddr, int len, bool is_end = false) {
	return self.RunVad(reinterpret_cast<const int32_t*>(jsHeapAddr), len, false);
}

static int SnowboyVad_RunVadF32(snowboy::SnowboyVad& self, long jsHeapAddr, int len, bool is_end = false) {
	return self.RunVad(reinterpret_cast<const float*>(jsHeapAddr), len, false);
}

static snowboy::SnowboyPersonalEnroll* makeEnroll(const std::string& res_filename, const std::string& model_str) {
	try {
		return new snowboy::SnowboyPersonalEnroll(res_filename, model_str);
	} catch (std::exception& e) {
		std::printf("Exception in SnowboyPersonalEnroll ctor: %s\n", e.what());
		throw;
	}
}

static int SnowboyPersonalEnroll_RunEnrollmentI16(snowboy::SnowboyPersonalEnroll& self, long jsHeapAddr, int len) {
	return self.RunEnrollment(reinterpret_cast<const int16_t*>(jsHeapAddr), len);
}

static int SnowboyPersonalEnroll_RunEnrollmentI32(snowboy::SnowboyPersonalEnroll& self, long jsHeapAddr, int len) {
	return self.RunEnrollment(reinterpret_cast<const int32_t*>(jsHeapAddr), len);
}

static int SnowboyPersonalEnroll_RunEnrollmentF32(snowboy::SnowboyPersonalEnroll& self, long jsHeapAddr, int len) {
	return self.RunEnrollment(reinterpret_cast<const float*>(jsHeapAddr), len);
}

static snowboy::SnowboyTemplateCut* makeCut(const std::string& res_filename) {
	try {
		return new snowboy::SnowboyTemplateCut(res_filename);
	} catch (std::exception& e) {
		std::printf("Exception in SnowboyTemplateCut ctor: %s\n", e.what());
		throw;
	}
}

static int SnowboyTemplateCut_CutTemplateI16(snowboy::SnowboyTemplateCut& self, long jsHeapAddrIn, int len_in, long jsHeapAddrOut, long jsHeapAddrLenOut) {
	return self.CutTemplate(
		reinterpret_cast<const int16_t*>(jsHeapAddrIn),
		len_in,
		reinterpret_cast<int16_t*>(jsHeapAddrOut),
		reinterpret_cast<int*>(jsHeapAddrLenOut));
}

static int SnowboyTemplateCut_CutTemplateI32(snowboy::SnowboyTemplateCut& self, long jsHeapAddrIn, int len_in, long jsHeapAddrOut, long jsHeapAddrLenOut) {
	return self.CutTemplate(
		reinterpret_cast<const int32_t*>(jsHeapAddrIn),
		len_in,
		reinterpret_cast<int32_t*>(jsHeapAddrOut),
		reinterpret_cast<int*>(jsHeapAddrLenOut));
}

static int SnowboyTemplateCut_CutTemplateF32(snowboy::SnowboyTemplateCut& self, long jsHeapAddrIn, int len_in, long jsHeapAddrOut, long jsHeapAddrLenOut) {
	return self.CutTemplate(
		reinterpret_cast<const float*>(jsHeapAddrIn),
		len_in,
		reinterpret_cast<float*>(jsHeapAddrOut),
		reinterpret_cast<int*>(jsHeapAddrLenOut));
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

	class_<snowboy::SnowboyPersonalEnroll>("SnowboyPersonalEnroll")
		.constructor(&makeEnroll, allow_raw_pointers())
		.function("SampleRate", &snowboy::SnowboyPersonalEnroll::SampleRate)
		.function("NumChannels", &snowboy::SnowboyPersonalEnroll::NumChannels)
		.function("BitsPerSample", &snowboy::SnowboyPersonalEnroll::BitsPerSample)
		.function("GetNumTemplates", &snowboy::SnowboyPersonalEnroll::GetNumTemplates)
		.function("Reset", &snowboy::SnowboyPersonalEnroll::Reset)
		.function("RunEnrollmentI16", &SnowboyPersonalEnroll_RunEnrollmentI16)
		.function("RunEnrollmentI32", &SnowboyPersonalEnroll_RunEnrollmentI32)
		.function("RunEnrollmentF32", &SnowboyPersonalEnroll_RunEnrollmentF32);

	class_<snowboy::SnowboyTemplateCut>("SnowboyTemplateCut")
		.constructor(&makeCut, allow_raw_pointers())
		.function("SampleRate", &snowboy::SnowboyTemplateCut::SampleRate)
		.function("NumChannels", &snowboy::SnowboyTemplateCut::NumChannels)
		.function("BitsPerSample", &snowboy::SnowboyTemplateCut::BitsPerSample)
		.function("Reset", &snowboy::SnowboyTemplateCut::Reset)
		.function("CutTemplateI16", &SnowboyTemplateCut_CutTemplateI16)
		.function("CutTemplateI32", &SnowboyTemplateCut_CutTemplateI32)
		.function("CutTemplateF32", &SnowboyTemplateCut_CutTemplateF32);
}
