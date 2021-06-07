#include <intercept-stream.h>
#include <matrix-wrapper.h>

namespace snowboy {

	InterceptStream::InterceptStream() {
	}

	int InterceptStream::Read(Matrix* mat, std::vector<FrameInfo>* info) {
		if (m_connectedStream) throw std::runtime_error("InterceptStream can not be connected");
		if (m_matrix_queue.empty()) {
			mat->Resize(0, 0);
			info->clear();
			return 0x100; // End of stream ?
		}
		*mat = m_matrix_queue.front();
		*info = m_frame_info_queue.front();
		auto res = m_signal_queue.front();
		m_matrix_queue.pop_front();
		m_frame_info_queue.pop_front();
		m_signal_queue.pop_front();
		return res;
	}

	bool InterceptStream::Reset() {
		m_matrix_queue.clear();
		m_frame_info_queue.clear();
		m_signal_queue.clear();
		return true;
	}

	std::string InterceptStream::Name() const {
		return "InterceptStream";
	}

	InterceptStream::~InterceptStream() {}

	void InterceptStream::ReadData(Matrix* mat, std::vector<FrameInfo>* info, SnowboySignal* signal) {
		*signal = static_cast<SnowboySignal>(m_connectedStream->Read(mat, info));
	}

	void InterceptStream::SetData(const MatrixBase& mat, const std::vector<FrameInfo>& info, const SnowboySignal& signal) {
		m_matrix_queue.push_back(Matrix{mat});
		m_frame_info_queue.push_back(info);
		m_signal_queue.push_back(signal);
	}
} // namespace snowboy
