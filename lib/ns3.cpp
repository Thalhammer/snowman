#include <cstdint>
#include <cstdlib>
#include <cstring>

extern "C"
{
	extern int TNRx_Create(void** param_1);
	extern int TNRx_Free(void* param_1);
	extern void TNRx_Init(void* param_1, int sample_rate);
	extern int TNRx_set_policy(void* param_1, long param_2);
	extern int TNRx_set_dereverb(void* param_1, long param_2);
	extern void TNRx_Process(void* param_1);

	struct NS3_Instance {
		void* m_tnrx_instance;
		short m_unknown;
	};
	//static_assert(sizeof(NS3_Instance) == 0x10);

	NS3_Instance* NS3_Init(int sample_rate /* probably */, int unknown1, int* unknown2 /* status? */) {
		if ((sample_rate == 8000 || sample_rate == 16000 || sample_rate == 32000 || sample_rate == 48000)
			&& (unknown1 == 0x50 || unknown1 == 0xa0 || unknown1 == 0x140 || unknown1 == 0x1e0)) {
			auto res = new NS3_Instance{};
			TNRx_Create(&res->m_tnrx_instance);
			TNRx_Init(res->m_tnrx_instance, sample_rate);
			TNRx_set_policy(res->m_tnrx_instance, 1);
			res->m_unknown = unknown1;
			*unknown2 = 1;
			return res;
		}
		return nullptr;
	}

	int NS3_Exit(NS3_Instance* instance) {
		TNRx_Free(instance->m_tnrx_instance);
		delete instance;
		return 1;
	}

	int NS3_Process(NS3_Instance* instance) {
		TNRx_Process(instance->m_tnrx_instance);
		return 1;
	}

	int NS3_SetPara(NS3_Instance* instance, const char* property, const char* value) {
		if (instance == nullptr) return 2;
		if (strcmp(property, "NS_Power") == 0) {
			auto val = strtol(value, nullptr, 10);
			TNRx_set_policy(instance->m_tnrx_instance, 1);
			if (TNRx_set_policy(instance->m_tnrx_instance, val & 0xffffffff) == -1) return 4;
		} else if (strcmp(property, "DR_Power") == 0) {
			auto val = strtol(value, nullptr, 10);
			TNRx_set_dereverb(instance->m_tnrx_instance, 0);
			if (TNRx_set_dereverb(instance->m_tnrx_instance, val & 0xffffffff) == -1) return 4;
		} else
			return 4;
		return 1;
	}
}
