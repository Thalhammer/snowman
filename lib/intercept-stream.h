#pragma once
#include <deque>
#include <frame-info.h>
#include <stream-itf.h>
#include <vector>

namespace snowboy {
	class InterceptStream : public StreamItf {
		std::deque<Matrix> m_matrix_queue;
		std::deque<std::vector<FrameInfo>> m_frame_info_queue;
		std::deque<SnowboySignal> m_signal_queue;

	public:
		InterceptStream();
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~InterceptStream();

		void ReadData(Matrix* mat, std::vector<FrameInfo>* info, SnowboySignal* signal);
		void SetData(const MatrixBase& mat, const std::vector<FrameInfo>& info, const SnowboySignal& signal);
	};
} // namespace snowboy
