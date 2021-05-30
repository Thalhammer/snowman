#include <eavesdrop-stream.h>
#include <frame-info.h>
#include <snowboy-error.h>

namespace snowboy {

	EavesdropStream::EavesdropStream(Matrix* data_ptr, std::vector<FrameInfo>* info_ptr) {
		if (data_ptr == nullptr && info_ptr == nullptr)
			throw snowboy_exception{"both data and info pointers are NULL, at least one of them should not be NULL"};
		m_data_ptr = data_ptr;
		m_info_ptr = info_ptr;
	}

	int EavesdropStream::Read(Matrix* mat, std::vector<FrameInfo>* info) {
		if (m_data_ptr == nullptr && m_info_ptr == nullptr)
			throw snowboy_exception{"both data and info pointers are NULL, at least one of them should not be NULL"};
		auto sig = m_connectedStream->Read(mat, info);
		if (m_data_ptr != nullptr) {
			*m_data_ptr = *mat;
		}
		if (m_info_ptr != nullptr) {
			*m_info_ptr = *info;
		}
		return sig;
	}

	bool EavesdropStream::Reset() {
		return true;
	}

	std::string EavesdropStream::Name() const {
		return "EavesdropStream";
	}

	EavesdropStream::~EavesdropStream() {}

} // namespace snowboy
