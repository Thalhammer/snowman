#pragma once
#include <stream-itf.h>
#include <memory>

namespace snowboy {
	struct OptionsItf;
    struct FftItf;
	struct FftStreamOptions {
		int num_fft_points;
		std::string method;
		void Register(const std::string& prefix, OptionsItf* options);
	};
	static_assert(sizeof(FftStreamOptions) == 0x10);
    struct FftStream: StreamItf {
		FftStreamOptions m_options;
        std::unique_ptr<FftItf> m_fft;
        int num_fft_points;

        void InitFft(int num_points);

        FftStream(const FftStreamOptions& options);
        virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~FftStream();
	};
	static_assert(sizeof(FftStream) == 0x38);

} // namespace snowboy