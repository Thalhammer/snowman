#pragma once
#include <deque>
#include <matrix-wrapper.h>
#include <stream-itf.h>

namespace snowboy {
	struct EavesdropStream : StreamItf {
		Matrix* m_data_ptr;
		std::vector<FrameInfo>* m_info_ptr;

		EavesdropStream(Matrix* data_ptr, std::vector<FrameInfo>* info_ptr);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~EavesdropStream();
	};
} // namespace snowboy