#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <snowboy-detect-c.h>
#include <snowboy-detect.h>

extern "C"
{
	void SNOWMAN_free(void* ptr) {
		free(ptr);
	}

	struct SNOWMAN_Detect : snowboy::SnowboyDetect {
		using SnowboyDetect::SnowboyDetect;
	};

	struct SNOWMAN_Vad : snowboy::SnowboyVad {
		using SnowboyVad::SnowboyVad;
	};

	struct SNOWMAN_PersonalEnroll : snowboy::SnowboyPersonalEnroll {
		using SnowboyPersonalEnroll::SnowboyPersonalEnroll;
	};

	struct SNOWMAN_TemplateCut : snowboy::SnowboyTemplateCut {
		using SnowboyTemplateCut::SnowboyTemplateCut;
	};

	SNOWMAN_Detect* SNOWMAN_Detect_Create(const char* resource_filename, const char* model_str) {
		if (resource_filename == nullptr) resource_filename = "common.res";
		if (model_str == nullptr) model_str = "model.umdl";
		try {
			return new SNOWMAN_Detect{resource_filename, model_str};
		} catch (...) {
			errno = EIO;
			return nullptr;
		}
	}

	int SNOWMAN_Detect_Reset(SNOWMAN_Detect* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->Reset() ? 1 : 0;
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Detect_RunDetectionWave(SNOWMAN_Detect* instance, const void* data, unsigned int len, int is_end) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		if (data == nullptr || len == 0) return 0;
		try {
			std::string temp{reinterpret_cast<const char*>(data), len};
			return instance->RunDetection(temp, is_end != 0);
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Detect_RunDetectionFloat(SNOWMAN_Detect* instance, const float* data, unsigned int num_samples, int is_end) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		if (data == nullptr || num_samples == 0) return 0;
		try {
			return instance->RunDetection(data, num_samples, is_end != 0);
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Detect_RunDetectionShort(SNOWMAN_Detect* instance, const short* data, unsigned int num_samples, int is_end) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		if (data == nullptr || num_samples == 0) return 0;
		try {
			return instance->RunDetection(data, num_samples, is_end != 0);
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Detect_RunDetectionInt(SNOWMAN_Detect* instance, const int* data, unsigned int num_samples, int is_end) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		if (data == nullptr || num_samples == 0) return 0;
		try {
			return instance->RunDetection(data, num_samples, is_end != 0);
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Detect_SetSensitivity(SNOWMAN_Detect* instance, const char* sensitivity) {
		if (instance == nullptr || sensitivity == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			instance->SetSensitivity(sensitivity);
			return 0;
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Detect_SetHighSensitivity(SNOWMAN_Detect* instance, const char* sensitivity) {
		if (instance == nullptr || sensitivity == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			instance->SetHighSensitivity(sensitivity);
			return 0;
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Detect_GetSensitivity(SNOWMAN_Detect* instance, char** pointer) {
		if (pointer == nullptr) return 0;
		if (instance == nullptr || *pointer != nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			auto s = instance->GetSensitivity();
			*pointer = static_cast<char*>(malloc(s.size() + 1));
			if (*pointer == nullptr) {
				errno = ENOMEM;
				return -1;
			}
			strcpy(*pointer, s.c_str());
			(*pointer)[s.size()] = '\0';
			return 0;
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Detect_SetAudioGain(SNOWMAN_Detect* instance, float gain) {
		if (instance == nullptr || gain <= 0) {
			errno = EINVAL;
			return -1;
		}
		try {
			instance->SetAudioGain(gain);
			return 0;
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Detect_UpdateModel(SNOWMAN_Detect* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			instance->UpdateModel();
			return 0;
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Detect_NumHotwords(SNOWMAN_Detect* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->NumHotwords();
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Detect_ApplyFrontend(SNOWMAN_Detect* instance, int apply) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			instance->ApplyFrontend(apply != 0);
			return 0;
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Detect_SampleRate(SNOWMAN_Detect* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->SampleRate();
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Detect_NumChannels(SNOWMAN_Detect* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->NumChannels();
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Detect_BitsPerSample(SNOWMAN_Detect* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->BitsPerSample();
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	void SNOWMAN_Detect_Destroy(SNOWMAN_Detect* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return;
		}
		try {
			delete instance;
		} catch (...) {
			// Should never happen, but better be safe than sorry
			errno = EIO;
			return;
		}
	}

	SNOWMAN_Vad* SNOWMAN_Vad_Create(const char* resource_filename) {
		if (resource_filename == nullptr) resource_filename = "common.res";
		try {
			return new SNOWMAN_Vad{resource_filename};
		} catch (...) {
			errno = EIO;
			return nullptr;
		}
	}

	int SNOWMAN_Vad_Reset(SNOWMAN_Vad* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->Reset() ? 1 : 0;
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Vad_RunVadWave(SNOWMAN_Vad* instance, const void* data, unsigned int len, int is_end) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		if (data == nullptr || len == 0) return 0;
		try {
			std::string temp{reinterpret_cast<const char*>(data), len};
			return instance->RunVad(temp, is_end != 0);
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Vad_RunVadFloat(SNOWMAN_Vad* instance, const float* data, unsigned int num_samples, int is_end) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		if (data == nullptr || num_samples == 0) return 0;
		try {
			return instance->RunVad(data, num_samples, is_end != 0);
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Vad_RunVadShort(SNOWMAN_Vad* instance, const short* data, unsigned int num_samples, int is_end) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		if (data == nullptr || num_samples == 0) return 0;
		try {
			return instance->RunVad(data, num_samples, is_end != 0);
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Vad_RunVadInt(SNOWMAN_Vad* instance, const int* data, unsigned int num_samples, int is_end) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		if (data == nullptr || num_samples == 0) return 0;
		try {
			return instance->RunVad(data, num_samples, is_end != 0);
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Vad_SetAudioGain(SNOWMAN_Vad* instance, float gain) {
		if (instance == nullptr || gain <= 0) {
			errno = EINVAL;
			return -1;
		}
		try {
			instance->SetAudioGain(gain);
			return 0;
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Vad_ApplyFrontend(SNOWMAN_Vad* instance, int apply) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			instance->ApplyFrontend(apply != 0);
			return 0;
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Vad_SampleRate(SNOWMAN_Vad* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->SampleRate();
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Vad_NumChannels(SNOWMAN_Vad* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->NumChannels();
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_Vad_BitsPerSample(SNOWMAN_Vad* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->BitsPerSample();
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	void SNOWMAN_Vad_Destroy(SNOWMAN_Vad* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return;
		}
		try {
			delete instance;
		} catch (...) {
			// Should never happen, but better be safe than sorry
			errno = EIO;
			return;
		}
	}

	SNOWMAN_PersonalEnroll* SNOWMAN_PersonalEnroll_Create(const char* resource_filename, const char* model_str) {
		if (resource_filename == nullptr) resource_filename = "common.res";
		if (model_str == nullptr) model_str = "model.pmdl";
		try {
			return new SNOWMAN_PersonalEnroll{resource_filename, model_str};
		} catch (...) {
			errno = EIO;
			return nullptr;
		}
	}

	int SNOWMAN_PersonalEnroll_Reset(SNOWMAN_PersonalEnroll* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->Reset() ? 1 : 0;
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_PersonalEnroll_RunEnrollmentWave(SNOWMAN_PersonalEnroll* instance, const void* data, unsigned int len) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		if (data == nullptr || len == 0) return 0;
		try {
			std::string temp{reinterpret_cast<const char*>(data), len};
			return instance->RunEnrollment(temp);
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_PersonalEnroll_RunEnrollmentFloat(SNOWMAN_PersonalEnroll* instance, const float* data, unsigned int num_samples) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		if (data == nullptr || num_samples == 0) return 0;
		try {
			return instance->RunEnrollment(data, num_samples);
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_PersonalEnroll_RunEnrollmentShort(SNOWMAN_PersonalEnroll* instance, const short* data, unsigned int num_samples) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		if (data == nullptr || num_samples == 0) return 0;
		try {
			return instance->RunEnrollment(data, num_samples);
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_PersonalEnroll_RunEnrollmentInt(SNOWMAN_PersonalEnroll* instance, const int* data, unsigned int num_samples) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		if (data == nullptr || num_samples == 0) return 0;
		try {
			return instance->RunEnrollment(data, num_samples);
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_PersonalEnroll_GetNumTemplates(SNOWMAN_PersonalEnroll* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->GetNumTemplates();
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_PersonalEnroll_SampleRate(SNOWMAN_PersonalEnroll* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->SampleRate();
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_PersonalEnroll_NumChannels(SNOWMAN_PersonalEnroll* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->NumChannels();
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_PersonalEnroll_BitsPerSample(SNOWMAN_PersonalEnroll* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->BitsPerSample();
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	void SNOWMAN_PersonalEnroll_Destroy(SNOWMAN_PersonalEnroll* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return;
		}
		try {
			delete instance;
		} catch (...) {
			// Should never happen, but better be safe than sorry
			errno = EIO;
			return;
		}
	}

	SNOWMAN_TemplateCut* SNOWMAN_TemplateCut_Create(const char* resource_filename) {
		if (resource_filename == nullptr) resource_filename = "common.res";
		try {
			return new SNOWMAN_TemplateCut{resource_filename};
		} catch (...) {
			errno = EIO;
			return nullptr;
		}
	}

	int SNOWMAN_TemplateCut_Reset(SNOWMAN_TemplateCut* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->Reset() ? 1 : 0;
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_TemplateCut_CutTemplateWave(SNOWMAN_TemplateCut* instance, const void* indata, unsigned int inlen, void** outdata, unsigned int* outlen) {
		if (indata == nullptr || inlen == 0 || outdata == nullptr || outlen == 0) return 0;
		if (instance == nullptr || *outdata != nullptr || *outlen != 0) {
			errno = EINVAL;
			return -1;
		}
		try {
			std::string temp{reinterpret_cast<const char*>(indata), inlen};
			auto res = instance->CutTemplate(temp);
			*outdata = static_cast<char*>(malloc(res.size()));
			if (*outdata == nullptr) {
				errno = ENOMEM;
				return -1;
			}
			memcpy(*outdata, res.data(), res.size());
			*outlen = res.size();
			return 0;
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_TemplateCut_SampleRate(SNOWMAN_TemplateCut* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->SampleRate();
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_TemplateCut_NumChannels(SNOWMAN_TemplateCut* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->NumChannels();
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	int SNOWMAN_TemplateCut_BitsPerSample(SNOWMAN_TemplateCut* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return -1;
		}
		try {
			return instance->BitsPerSample();
		} catch (...) {
			errno = EIO;
			return -1;
		}
	}

	void SNOWMAN_TemplateCut_Destroy(SNOWMAN_TemplateCut* instance) {
		if (instance == nullptr) {
			errno = EINVAL;
			return;
		}
		try {
			delete instance;
		} catch (...) {
			// Should never happen, but better be safe than sorry
			errno = EIO;
			return;
		}
	}
}
