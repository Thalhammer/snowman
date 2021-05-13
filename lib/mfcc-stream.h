#pragma once
#include <feat-lib.h>
#include <matrix-wrapper.h>
#include <memory>
#include <stream-itf.h>

namespace snowboy {
	struct MfccStreamOptions {
		MelFilterBankOptions mel_filter;
		int num_cepstral_coeffs;
		bool use_energy;
		float cepstral_lifter;

		void Register(const std::string& prefix, OptionsItf* opts);
	};
	static_assert(sizeof(MfccStreamOptions) == 0x2c);
	struct MfccStream : StreamItf {
		MfccStreamOptions m_options;
		int field_x44;
		float field_x48;
		// Padding ?
		std::unique_ptr<MelFilterBank> m_melfilterbank;
		Matrix m_dct_matrix;
		Vector m_cepstral_coeffs;

		MfccStream(const MfccStreamOptions& options);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~MfccStream();
		void InitMelFilterBank(int);
		void ComputeMfcc(const VectorBase&, SubVector*) const;
	};
	static_assert(sizeof(MfccStream) == 0x80);
} // namespace snowboy