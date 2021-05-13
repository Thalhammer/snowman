#pragma once
#include <string>
#include <vector>

namespace snowboy {
	struct FrameInfo;
	struct MatrixBase;
	struct Matrix;
	struct StreamItf {
		// vtable ptr
		bool m_isConnected{false};
		StreamItf* m_connectedStream{nullptr};

		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) = 0;
		virtual bool Reset() = 0;
		virtual std::string Name() const = 0;
		virtual bool Connect(StreamItf* other);
		virtual bool Disconnect();
		virtual ~StreamItf();
	};
} // namespace snowboy