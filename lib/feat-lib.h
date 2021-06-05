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
		void ComputeMelFilterBankEnergy(const VectorBase& input, const VectorBase& output) const;

		const MelFilterBankOptions& get_options() const noexcept { return m_options; }
	};

	void ComputeDctMatrixTypeIII(const MatrixBase& mat);
	void ComputeCepstralLifterCoeffs(float, Vector*);
	/**
	 * Compute the Power Spectrum of a vector.
	 * out is expected to be at least half the size of in and can alias with in.
	 * \param in Vector to calculate spectrum of
	 * \param out Vector to place the result in.
	 */
	void ComputePowerSpectrumReal(const VectorBase& in, const VectorBase& out);

	struct FftItf {
		virtual void DoFft(Vector*) const noexcept = 0;
		virtual void DoIfft(Vector*) const noexcept = 0;
		virtual ~FftItf();
	};

	struct FftOptions {
		bool field_x00;
		int num_fft_points;
	};

	class Fft : public FftItf {
		FftOptions m_options;
		std::vector<unsigned int> m_bit_reversal_index;
		std::vector<float> m_twiddle_factors;

		void DoFft(bool inverse, Vector*) const noexcept;
		void DoDanielsonLanczos(bool inverse, const VectorBase& data) const noexcept;
		static void DoBitReversalSorting(const std::vector<unsigned int>&, const VectorBase& data) noexcept;
		void ComputeTwiddleFactor(size_t len);
		void ComputeBitReversalIndex(size_t len);
		void DoProcessingForReal(bool, Vector*) const noexcept;
		static size_t GetNumBits(size_t) noexcept;
		std::pair<float, float> GetTwiddleFactor(int, int) const noexcept;
		void Init();
		static size_t ReverseBit(size_t value, size_t num_bits) noexcept;
		void SetOptions(const FftOptions& opts);

	public:
		Fft(const FftOptions& options);
		virtual void DoFft(Vector* data) const noexcept override;
		virtual void DoIfft(Vector* data) const noexcept override;
		virtual ~Fft();
	};

} // namespace snowboy
