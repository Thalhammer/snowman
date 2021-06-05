#pragma once
#include <feat-lib.h>

namespace snowboy {

	struct SplitRadixFft : FftItf {
		FftOptions m_options;
		size_t field_x10;
		int field_x14;
		std::vector<int> field_x18;
		std::vector<std::vector<float>> field_x30;

		SplitRadixFft(const FftOptions& options);
		void DoFft(bool inverse, Vector* data) const;
		void DoComplexFftRecursive(int, float*, float*) const;
		void DoComplexFftComputation(bool, float*, float*) const;
		void DoComplexFft(bool, Vector*) const;
		void ComputeTables();
		void BitReversePermute(int, float*) const;
		void DoProcessingForReal(bool, Vector*) const;
		void Init();
		void SetOptions(const FftOptions& options);

		virtual void DoFft(Vector*) const noexcept override;
		virtual void DoIfft(Vector*) const noexcept override;
		virtual ~SplitRadixFft();
	};
	//static_assert(sizeof(SplitRadixFft) == 0x48);
} // namespace snowboy
