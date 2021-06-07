#include <cstdint>
#include <cstdlib>
#include <cstring>

extern "C"
{
	// not sure if those are correct
	struct TAgc_config {
		short level;
		short power;
	};
	struct TAgc_Instance {
		unsigned char unknown1[8];
		short unknown2;
		unsigned char unknown3[0x28e];
	};
	//static_assert(sizeof(TAgc_Instance) == 0x298);
	// TODO: Can we just copy the WebRtc implementation ?
	// https://chromium.googlesource.com/external/webrtc/+/refs/heads/master/modules/audio_processing/agc/legacy/gain_control.h
	extern void TAgc_Create(TAgc_Instance** ptr);
	extern void TAgc_Init_org(TAgc_Instance* instance, uint64_t, uint64_t, int, int sample_rate);
	extern void TAgc_Free(TAgc_Instance* instance);
	extern int TAgc_set_config(TAgc_Instance* instance, TAgc_config cfg);
	extern int TAgc_AddMic(TAgc_Instance* instance, long, long, short);
	extern int TAgc_VirtualMic(TAgc_Instance* instance, long, long, short, int, int*);
	extern int TAgc_Process(TAgc_Instance*, long, long, short, long, long, long, long*, long, long*);

	int agc_targetlevel = 4;
	int agc_targetpower = 5;

	struct AGC_Instance {
		TAgc_Instance* m_tagc_instance;
		short unknown1;
		uint32_t unknown2;
		uint32_t unknown3;
		// 4 more bytes, probably padding
	};
	//static_assert(sizeof(AGC_Instance) == 0x18);

	AGC_Instance* AGC_Init(int sample_rate /* probably */, int param_2, short param_3, int* param_4 /* status? */) {
		if ((sample_rate == 8000 || sample_rate == 16000 || sample_rate == 32000 || sample_rate == 48000)
			&& (param_2 == 0x50 || param_2 == 0xa0 || param_2 == 0x140 || param_2 == 0x1e0)) {
			auto res = new AGC_Instance{};
			TAgc_Create(&res->m_tagc_instance);
			TAgc_Init_org(res->m_tagc_instance, 0, 0xff, param_3 + 1, sample_rate);
			if (sample_rate == 32000 || sample_rate == 48000)
				res->unknown1 = 0xa0;
			else
				res->unknown1 = param_2;
			*param_4 = 1;
			res->unknown2 = 0;
			res->unknown3 = 0;
			return res;
		}
		*param_4 = 4;
		return nullptr;
	}

	int AGC_Exit(AGC_Instance* instance) {
		TAgc_Free(instance->m_tagc_instance);
		delete instance;
		return 1;
	}

	int AGC_Process(AGC_Instance* instance, long param_2, long param_3, long param_4, long param_5, int param_6) {
		instance->m_tagc_instance->unknown2 = param_6 + 1;
		if (param_6 == 0) {
			TAgc_AddMic(instance->m_tagc_instance, param_2, param_3, instance->unknown1);
			long a = 0, b = 0;
			TAgc_Process(instance->m_tagc_instance, param_2, param_3, instance->unknown1, param_4, param_5, 0, &a, 0, &b);
			instance->unknown3 = b;
			instance->unknown2 = a;
		} else {
			int a = 0;
			if (param_6 == 1) {
				TAgc_VirtualMic(instance->m_tagc_instance, param_2, param_3, instance->unknown1, instance->unknown2, &a);
			}
			long b = 0, c = 0;
			TAgc_Process(instance->m_tagc_instance, param_2, param_3, instance->unknown1, param_4, param_5, a, &b, 0, &c);
			instance->unknown3 = c;
		}
		return 1;
	}

	int AGC_SetPara(AGC_Instance* instance, const char* property, const char* value) {
		if (instance == nullptr) return 2;
		TAgc_config cfg;
		cfg.level = agc_targetlevel;
		cfg.power = agc_targetpower;
		if (strcmp(property, "AGC_Level") == 0) {
			auto val = strtol(value, nullptr, 10);
			cfg.level = agc_targetlevel = val;
		} else if (strcmp(property, "AGC_Power") == 0) {
			auto val = strtol(value, nullptr, 10);
			cfg.power = agc_targetpower = val;
		} else
			return 4;
		auto res = TAgc_set_config(instance->m_tagc_instance, cfg);
		if (res == -1) return 4;
		return 1;
	}
}
