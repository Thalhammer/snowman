#pragma once

extern "C"
{
	struct NS3_Instance;
	NS3_Instance* NS3_Init(int sample_rate /* probably */, int unknown1, int* unknown2 /* status? */);
	int NS3_Exit(NS3_Instance* instance);
	int NS3_Process(NS3_Instance* instance);
	int NS3_SetPara(NS3_Instance* instance, const char* property, const char* value);
}