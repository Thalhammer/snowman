#pragma once

extern "C"
{
	struct AGC_Instance;
	AGC_Instance* AGC_Init(int sample_rate /* probably */, int param_2, short param_3, int* param_4 /* status? */);
	int AGC_Exit(AGC_Instance* instance);
	int AGC_Process(AGC_Instance* instance);
	int AGC_SetPara(AGC_Instance* instance, const char* property, const char* value);
}