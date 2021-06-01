#include <frame-info.h>
#include <nnet-lib.h>
#include <raw-nnet-vad-stream.h>
#include <snowboy-error.h>
#include <snowboy-io.h>
#include <snowboy-options.h>

namespace snowboy {
	void RawNnetVadStreamOptions::Register(const std::string& prefix, OptionsItf* opts) {
		opts->Register(prefix, "non-voice-index", "Index of the non-voice label in the neural network model output.", &non_voice_index);
		opts->Register(prefix, "non-voice-threshold", "Frames with non-voice probability higher than the given threshold will be treated as a non-voice frame.", &non_voice_threshold);
		opts->Register(prefix, "model-filename", "Filename of the neural network VAD model.", &model_filename);
	}

	RawNnetVadStream::RawNnetVadStream(const RawNnetVadStreamOptions& options) {
		m_options = options;
		if (m_options.model_filename == "")
			throw snowboy_exception{"please specify the neural network VAD model."};
		m_nnet.reset(new Nnet(true));
		Input model{m_options.model_filename};
		m_nnet->Read(model.is_binary(), model.Stream());
		auto dims = m_nnet->OutputDim();
		if (dims <= m_options.non_voice_index || m_options.non_voice_index < 0)
			throw snowboy_exception{"index " + std::to_string(m_options.non_voice_index)
									+ " for non-voice label runs out of range (0 - " + std::to_string(dims) + "), wrong index?"};
	}

	int RawNnetVadStream::Read(Matrix* mat, std::vector<FrameInfo>* info) {
		Matrix tmat;
		std::vector<FrameInfo> tinfo;
		auto sig = m_connectedStream->Read(&tmat, &tinfo);
		if ((sig & 0xc2) != 0) {
			mat->Resize(0, 0);
			info->clear();
			return sig;
		}
		Matrix nnet_output;
		if ((sig & 0x18) == 0) {
			m_nnet->Compute(tmat, tinfo, &nnet_output, info);
		} else {
			m_nnet->FlushOutput(tmat, tinfo, &nnet_output, info);
		}
		auto cols = m_fieldx30.m_cols;
		if (cols < 1) cols = tmat.m_cols;
		Matrix m;
		m.Resize(tmat.m_rows + m_fieldx30.m_rows, cols);
		if (m_fieldx30.m_rows > 0) {
			m.RowRange(0, m_fieldx30.m_rows).CopyFromMat(m_fieldx30, MatrixTransposeType::kNoTrans);
		}
		if (tmat.m_rows > 0) {
			m.RowRange(m_fieldx30.m_rows, tmat.m_rows).CopyFromMat(tmat, MatrixTransposeType::kNoTrans);
		}
		if (nnet_output.m_rows < 1) {
			mat->Resize(0, 0);
		} else {
			mat->Resize(nnet_output.m_rows, m.m_cols, MatrixResizeType::kUndefined);
			mat->CopyFromMat(m.RowRange(0, nnet_output.m_rows), MatrixTransposeType::kNoTrans);
		}
		if (m.m_rows - nnet_output.m_rows < 1) {
			m_fieldx30.Resize(0, 0);
		} else {
			m_fieldx30.Resize(m.m_rows - nnet_output.m_rows, m.m_cols, MatrixResizeType::kUndefined);
			m_fieldx30.CopyFromMat(m.RowRange(nnet_output.m_rows, m.m_cols - nnet_output.m_rows), MatrixTransposeType::kNoTrans);
		}
		for (size_t r = 0; r < nnet_output.rows(); r++) {
			auto f = nnet_output(r, m_options.non_voice_index);
			if (f <= m_options.non_voice_threshold) {
				info->at(r).flags |= 0x1;
			} else
				info->at(r).flags &= ~0x1;
		}
		return sig;
	}

	bool RawNnetVadStream::Reset() {
		m_nnet->ResetComputation();
		m_fieldx30.Resize(0, 0);
		return true;
	}

	std::string RawNnetVadStream::Name() const {
		return "RawNnetVadStream";
	}

	RawNnetVadStream::~RawNnetVadStream() {}

} // namespace snowboy
