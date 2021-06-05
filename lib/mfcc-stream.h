#pragma once
#include <feat-lib.h>
#include <matrix-wrapper.h>
#include <memory>
#include <stream-itf.h>

namespace snowboy {
	struct MfccStreamOptions {
		MelFilterBankOptions mel_filter;
		int num_cepstral_coeffs;
		float cepstral_lifter;
		bool use_energy;

		void Register(const std::string& prefix, OptionsItf* opts);
	};
	class MfccStream : public StreamItf {
		MfccStreamOptions m_options;
		size_t m_num_fft_points;
		float field_x48;
		std::unique_ptr<MelFilterBank> m_melfilterbank;
		Matrix m_dct_matrix;
		Vector m_cepstral_coeffs;
		void InitMelFilterBank(size_t num_fft_points);
		void ComputeMfcc(const VectorBase&, SubVector*) const;

	public:
		MfccStream(const MfccStreamOptions& options);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~MfccStream();
	};
} // namespace snowboy
