#pragma once
#include <frame-info.h>
#include <stream-itf.h>
#include <string>
#include <vector-wrapper.h>

namespace snowboy {
	struct OptionsItf;
	struct FramerStreamOptions {
		int sample_rate;
		int frame_length_ms;
		int frame_shift_ms;
		float dither_coeff;
		float preemphasis_coeff;
		bool subtract_mean;
		std::string window_type;
		void Register(const std::string&, OptionsItf*);
	};
	struct FramerStream : StreamItf {
		FramerStreamOptions m_options;
		int field_x38;
		int field_x3c; // might be padding
		Vector field_x40;
		size_t m_frame_shift_samples;
		size_t m_frame_length_samples;
		Vector m_window;

		void CreateWindow();
		void CreateFrames(const VectorBase& data, Matrix* mat);
		size_t NumFrames(size_t p1) const;

		FramerStream(const FramerStreamOptions& options);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~FramerStream();
	};
} // namespace snowboy
