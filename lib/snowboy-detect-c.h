#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

	extern void SNOWMAN_free(void*);

	struct SNOWMAN_Detect;
	struct SNOWMAN_Vad;
	struct SNOWMAN_PersonalEnroll;
	struct SNOWMAN_TemplateCut;

	SNOWMAN_Detect* SNOWMAN_Detect_Create(const char* resource_filename, const char* model_str);
	int SNOWMAN_Detect_Reset(SNOWMAN_Detect* instance);
	int SNOWMAN_Detect_RunDetectionWave(SNOWMAN_Detect* instance, const void* data, unsigned int len, int is_end);
	int SNOWMAN_Detect_RunDetectionFloat(SNOWMAN_Detect* instance, const float* data, unsigned int num_samples, int is_end);
	int SNOWMAN_Detect_RunDetectionShort(SNOWMAN_Detect* instance, const short* data, unsigned int num_samples, int is_end);
	int SNOWMAN_Detect_RunDetectionInt(SNOWMAN_Detect* instance, const int* data, unsigned int num_samples, int is_end);
	int SNOWMAN_Detect_SetSensitivity(SNOWMAN_Detect* instance, const char* sensitivity);
	int SNOWMAN_Detect_SetHighSensitivity(SNOWMAN_Detect* instance, const char* sensitivity);
	// Returned pointer needs to get freed using SNOWMAN_free
	int SNOWMAN_Detect_GetSensitivity(SNOWMAN_Detect* instance, char** pointer);
	int SNOWMAN_Detect_SetAudioGain(SNOWMAN_Detect* instance, float gain);
	int SNOWMAN_Detect_UpdateModel(SNOWMAN_Detect* instance);
	int SNOWMAN_Detect_NumHotwords(SNOWMAN_Detect* instance);
	int SNOWMAN_Detect_ApplyFrontend(SNOWMAN_Detect* instance, int apply);
	int SNOWMAN_Detect_SampleRate(SNOWMAN_Detect* instance);
	int SNOWMAN_Detect_NumChannels(SNOWMAN_Detect* instance);
	int SNOWMAN_Detect_BitsPerSample(SNOWMAN_Detect* instance);
	void SNOWMAN_Detect_Destroy(SNOWMAN_Detect* instance);

	SNOWMAN_Vad* SNOWMAN_Vad_Create(const char* resource_filename);
	int SNOWMAN_Vad_Reset(SNOWMAN_Vad* instance);
	int SNOWMAN_Vad_RunVadWave(SNOWMAN_Vad* instance, const void* data, unsigned int len, int is_end);
	int SNOWMAN_Vad_RunVadFloat(SNOWMAN_Vad* instance, const float* data, unsigned int num_samples, int is_end);
	int SNOWMAN_Vad_RunVadShort(SNOWMAN_Vad* instance, const short* data, unsigned int num_samples, int is_end);
	int SNOWMAN_Vad_RunVadInt(SNOWMAN_Vad* instance, const int* data, unsigned int num_samples, int is_end);
	int SNOWMAN_Vad_SetAudioGain(SNOWMAN_Vad* instance, float gain);
	int SNOWMAN_Vad_ApplyFrontend(SNOWMAN_Vad* instance, int apply);
	int SNOWMAN_Vad_SampleRate(SNOWMAN_Vad* instance);
	int SNOWMAN_Vad_NumChannels(SNOWMAN_Vad* instance);
	int SNOWMAN_Vad_BitsPerSample(SNOWMAN_Vad* instance);
	void SNOWMAN_Vad_Destroy(SNOWMAN_Vad* instance);

	SNOWMAN_PersonalEnroll* SNOWMAN_PersonalEnroll_Create(const char* resource_filename, const char* model_str);
	int SNOWMAN_PersonalEnroll_Reset(SNOWMAN_PersonalEnroll* instance);
	int SNOWMAN_PersonalEnroll_RunEnrollmentWave(SNOWMAN_PersonalEnroll* instance, const void* data, unsigned int len);
	int SNOWMAN_PersonalEnroll_RunEnrollmentFloat(SNOWMAN_PersonalEnroll* instance, const float* data, unsigned int num_samples);
	int SNOWMAN_PersonalEnroll_RunEnrollmentShort(SNOWMAN_PersonalEnroll* instance, const short* data, unsigned int num_samples);
	int SNOWMAN_PersonalEnroll_RunEnrollmentInt(SNOWMAN_PersonalEnroll* instance, const int* data, unsigned int num_samples);
	int SNOWMAN_PersonalEnroll_GetNumTemplates(SNOWMAN_PersonalEnroll* instance);
	int SNOWMAN_PersonalEnroll_SampleRate(SNOWMAN_PersonalEnroll* instance);
	int SNOWMAN_PersonalEnroll_NumChannels(SNOWMAN_PersonalEnroll* instance);
	int SNOWMAN_PersonalEnroll_BitsPerSample(SNOWMAN_PersonalEnroll* instance);
	void SNOWMAN_PersonalEnroll_Destroy(SNOWMAN_PersonalEnroll* instance);

	SNOWMAN_TemplateCut* SNOWMAN_TemplateCut_Create(const char* resource_filename);
	int SNOWMAN_TemplateCut_Reset(SNOWMAN_TemplateCut* instance);
	// Returned data needs to get freed using SNOWMAN_free
	int SNOWMAN_TemplateCut_CutTemplateWave(SNOWMAN_TemplateCut* instance, const void* indata, unsigned int inlen, void** outdata, unsigned int* outlen);
	int SNOWMAN_TemplateCut_SampleRate(SNOWMAN_TemplateCut* instance);
	int SNOWMAN_TemplateCut_NumChannels(SNOWMAN_TemplateCut* instance);
	int SNOWMAN_TemplateCut_BitsPerSample(SNOWMAN_TemplateCut* instance);
	void SNOWMAN_TemplateCut_Destroy(SNOWMAN_TemplateCut* instance);

#ifdef __cplusplus
}
#endif
