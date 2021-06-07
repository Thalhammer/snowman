#pragma once
#include <memory>
#include <stream-itf.h>
#include <string>

namespace snowboy {
	struct OptionsItf;
	class Nnet;
	struct NnetStreamOptions {
		std::string model_filename;
		bool pad_context;
		void Register(const std::string&, OptionsItf*);
	};
	class NnetStream : public StreamItf {
		NnetStreamOptions m_options;
		std::unique_ptr<Nnet> m_nnet;

	public:
		NnetStream(const NnetStreamOptions& options);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~NnetStream();
	};
} // namespace snowboy
