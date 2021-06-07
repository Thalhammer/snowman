#pragma once
#include <deque>
#include <matrix-wrapper.h>
#include <memory>
#include <stream-itf.h>

struct AGC_Instance;
struct NS3_Instance;
namespace snowboy {
	struct OptionsItf;
	class Nnet;
	struct RawNnetVadStreamOptions {
		int non_voice_index;
		float non_voice_threshold;
		std::string model_filename;
		void Register(const std::string&, OptionsItf*);
	};
	struct RawNnetVadStream : StreamItf {
		RawNnetVadStreamOptions m_options;
		std::unique_ptr<Nnet> m_nnet;
		// TODO: Do we need this ?
		Matrix m_fieldx30;

		RawNnetVadStream(const RawNnetVadStreamOptions& options);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~RawNnetVadStream();
	};
} // namespace snowboy
