#pragma once
#include <string>
#include <vector-wrapper.h>
#include <vector>

namespace snowboy {
	struct Matrix;
	struct OptionsItf;
	struct MelFilterBankOptions {
		uint32_t num_bins;
		uint32_t num_fft_points;
		uint32_t sample_rate;
		float low_frequency;
		float high_frequency;
		float vtln_low_frequency;
		float vtln_high_frequency;
		float vtln_warping_factor;

		void Register(const std::string& prefix, OptionsItf* opts);
	};

	class MelFilterBank {
		MelFilterBankOptions m_options;
		// Both hold num_bins entries
		std::vector<int> field_x28;
		std::vector<Vector> field_x40;

		void InitMelFilterBank();
		float GetVtlnWarping(float) const;
		void ValidateOptions() const;

	public:
		MelFilterBank(const MelFilterBankOptions& options);
		~MelFilterBank() {}
		void ComputeMelFilterBankEnergy(const VectorBase& input, Vector& output) const;
	};

	void ComputeDctMatrixTypeIII(Matrix* mat);
	void ComputeCepstralLifterCoeffs(float, Vector*);
	void ComputePowerSpectrumReal(Vector&);

	struct FftItf {
		virtual void DoFft(Vector*) const = 0;
		virtual void DoIfft(Vector*) const = 0;
		virtual ~FftItf();
	};

	struct FftOptions {
		bool field_x00;
		int num_fft_points;
	};

	struct Fft : FftItf {
		FftOptions m_options;
		int field_x10;
		std::vector<unsigned int> m_bit_reversal_index;
		std::vector<float> m_twiddle_factors; // twiddle factors ?

		Fft(const FftOptions& options);
		void DoFft(bool inverse, Vector*) const;
		void DoDanielsonLanczos(bool, Vector*) const;
		void DoBitReversalSorting(const std::vector<unsigned int>&, Vector*) const;
		void ComputeTwiddleFactor(int);
		void ComputeBitReversalIndex(int, std::vector<unsigned int>*) const;
		void DoProcessingForReal(bool, Vector*) const;
		unsigned int GetNumBits(unsigned int) const;
		std::pair<float, float> GetTwiddleFactor(int, int) const;
		void Init();
		unsigned int ReverseBit(unsigned int, unsigned int) const;
		void SetOptions(const FftOptions& opts);

		virtual void DoFft(Vector*) const override;
		virtual void DoIfft(Vector*) const override;
		virtual ~Fft();
	};

} // namespace snowboy
