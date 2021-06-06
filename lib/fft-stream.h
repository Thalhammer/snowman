#pragma once
#include <memory>
#include <stream-itf.h>

namespace snowboy {
	struct OptionsItf;
	struct FftItf;
	struct FftStreamOptions {
		int num_fft_points;
		std::string method;
		void Register(const std::string& prefix, OptionsItf* options);
	};
	class FftStream : public StreamItf {
		FftStreamOptions m_options;
		std::unique_ptr<FftItf> m_fft;
		int num_fft_points;

		void InitFft(int num_points);

	public:
		FftStream(const FftStreamOptions& options);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~FftStream();
	};

} // namespace snowboy
