#include <cstdint>
#include <cstdlib>

extern "C"
{

	extern void TNRx_InitCore(void* param_1, int sample_rate);
	extern int TNRx_set_policy_core(void* param_1, long param_2);
	extern int TNRx_set_dereverb_core(void* param_1, long param_2);
	extern void TNRx_ProcessCore(void* param_1);
	extern void Delete_TDereverb_x_Params(void*);

	int TNRx_Create(void** param_1) {
		*param_1 = malloc(0x35f8);
		if (*param_1 == nullptr) return -1;
		reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(*param_1) + 0xc48)[0] = 0;
		return 0;
	}

	int TNRx_Free(void* param_1) {
		if (param_1 == nullptr) return -1;
		Delete_TDereverb_x_Params(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(param_1) + 0x3550));
		free(param_1);
		return 0;
	}

	void TNRx_Init(void* param_1, int sample_rate) {
		TNRx_InitCore(param_1, sample_rate);
	}

	int TNRx_set_policy(void* param_1, long param_2) {
		return TNRx_set_policy_core(param_1, param_2);
	}

	int TNRx_set_dereverb(void* param_1, long param_2) {
		return TNRx_set_dereverb_core(param_1, param_2);
	}

	void TNRx_Process(void* param_1) {
		TNRx_ProcessCore(param_1);
	}
}