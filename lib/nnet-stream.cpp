#include <frame-info.h>
#include <nnet-lib.h>
#include <nnet-stream.h>
#include <snowboy-error.h>
#include <snowboy-io.h>
#include <snowboy-options.h>

namespace snowboy {

	void NnetStreamOptions::Register(const std::string& prefix, OptionsItf* opts) {
		opts->Register(prefix, "model-filename", "Filename of the neural network model.", &model_filename);
		opts->Register(prefix, "pad-context", "It true pad left and right context when necessary.", &pad_context);
	}

	NnetStream::NnetStream(const NnetStreamOptions& options) {
		m_options = options;
		if (m_options.model_filename == "")
			throw snowboy_exception{"please specify the neural network model"};
		m_nnet.reset(new Nnet(m_options.pad_context));

		Input model{m_options.model_filename};
		m_nnet->Read(model.is_binary(), model.Stream());
	}

	int NnetStream::Read(Matrix* mat, std::vector<FrameInfo>* info) {
		Matrix tmat;
		tmat.Resize(0, 0);
		std::vector<FrameInfo> tinfo;
		auto res = m_connectedStream->Read(&tmat, &tinfo);
		if ((res & 0xc2) != 0) {
			mat->Resize(0, 0);
			return res;
		}
		SNOWBOY_ASSERT(!tmat.HasNan() && !tmat.HasInfinity());
		if ((res & 0x18) == 0) {
			m_nnet->Compute(tmat, tinfo, mat, info);
		} else {
			m_nnet->FlushOutput(tmat, tinfo, mat, info);
		}
		return res;
	}

	bool NnetStream::Reset() {
		m_nnet->ResetComputation();
		return true;
	}

	std::string NnetStream::Name() const {
		return "NnetStream";
	}

	NnetStream::~NnetStream() {}

} // namespace snowboy
