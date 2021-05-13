#pragma once
#include <cstddef>
#include <cstdint>

extern "C"
{
	int32_t spx_exp2(int16_t x);
	int32_t spx_exp(int16_t x);
	int16_t TSpl_AddSatW16(int16_t a, int16_t b);
	int16_t TSpl_SatW32ToW16(int32_t value32);
	int TSpl_ComplexFFT(int16_t* frfi, int stages, int mode);
	int TSpl_ComplexIFFT(int16_t* frfi, int stages, int mode);
	uint32_t TSpl_DivU32U16(uint32_t a, uint16_t b);
	int32_t TSpl_DivW32W16(int32_t a, int16_t b);
	int16_t TSpl_DivW32W16ResW16(int32_t a, int16_t b);
	// TODO: DownsampleBy2
	int32_t TSpl_Energy(int16_t* vector, size_t vector_length, int* scale_factor);
	int16_t TSpl_GetScalingSquare(int16_t* in_vector, size_t in_vector_length, size_t times);
	int16_t TSpl_GetSizeInBits(uint32_t n);
	int16_t TSpl_MaxAbsValueW16(const int16_t* vector, size_t length);
	int16_t TSpl_MaxValueW16(const int16_t* vector, size_t length);
	int16_t TSpl_NormU32(uint32_t a);
	int16_t TSpl_NormW16(int16_t a);
	int16_t TSpl_NormW32(int32_t a);
	int32_t TSpl_Sqrt(int32_t value);
	int32_t TSpl_SqrtLocal(int32_t in);
}