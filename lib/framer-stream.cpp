#include <cmath>
#include <framer-stream.h>
#include <matrix-wrapper.h>
#include <random>
#include <snowboy-debug.h>
#include <snowboy-options.h>

namespace snowboy {
	void FramerStreamOptions::Register(const std::string& prefix, OptionsItf* opts) {
		opts->Register(prefix, "sample-rate", "Sampling rate.", &sample_rate);
		opts->Register(prefix, "frame-length", "Frame length in milliseconds.", &frame_length_ms);
		opts->Register(prefix, "frame-shift", "Frame shift in milliseconds.", &frame_shift_ms);
		opts->Register(prefix, "dither-coeff", "Dithering coefficient, 0 means no dithering at all.", &dither_coeff);
		opts->Register(prefix, "preemphasis-coeff", "Pre-emphasis coefficient.", &preemphasis_coeff);
		opts->Register(prefix, "subtract-mean", "If true, subtract mean from each frame.", &subtract_mean);
		opts->Register(prefix, "window-type", "Type of window to use, candidates are: hamming|hanning|rectangular|povey.", &window_type);
	}

	FramerStream::FramerStream(const FramerStreamOptions& options)
		: m_sample_rate{options.sample_rate}, m_frame_length_ms{options.frame_length_ms}, m_frame_shift_ms{options.frame_shift_ms},
		  m_dither_coeff{options.dither_coeff}, m_preemphasis_coeff{options.preemphasis_coeff}, m_subtract_mean{options.subtract_mean},
		  m_window_type{options.window_type} {
		const auto frames_per_ms = static_cast<double>(m_sample_rate) * 0.001;
		this->m_frame_length_samples = this->m_frame_length_ms * frames_per_ms;
		this->m_frame_shift_samples = this->m_frame_shift_ms * frames_per_ms;
		CreateWindow();
		this->field_x38 = 1;
	}

	void FramerStream::CreateWindow() {
		auto len = this->m_frame_length_samples;
		this->m_window.Resize(len);
		auto data = this->m_window.m_data;
		if (m_window_type == "hamming") {
			for (size_t i = 0; i < len; i++) {
				data[i] = 0.54 - 0.46 * cos((M_2_PI * i) / (len - 1));
			}
		} else if (m_window_type == "hanning") {
			for (size_t i = 0; i < len; i++) {
				data[i] = (1.0 - cos((M_2_PI * i) / (len - 1))) * 0.5;
			}
		} else if (m_window_type == "rectangular") {
			// TODO: Implement this correctly. Snowboy does not seem to follow the plain implementation on wikipedia
			for (size_t i = 0; i < len; i++) {
				data[i] = 1.0;
			}
		} else if (m_window_type == "povey") {
			for (size_t i = 0; i < len; i++) {
				auto v = cos((M_2_PI * (i + 1)) / (len - 1));
				v = pow((1.0 - v) * 0.5, 0.85);
				data[i] = v;
			}
		} else {
			SNOWBOY_ERROR() << "Window type " << m_window_type << " is not defined";
		}
	}

	void FramerStream::CreateFrames(const VectorBase& data, Matrix* mat) {
		const auto nframes = NumFrames(data.m_size);
		mat->Resize(nframes, this->m_frame_length_samples);
		std::mt19937 gen;
		// This might have a different mean
		std::normal_distribution<float> dist;
		for (int currentFrame = 0; currentFrame < nframes; currentFrame++) {
			SubVector sub{*mat, currentFrame};
			sub.CopyFromVec(data.Range(this->m_frame_shift_samples * currentFrame, this->m_frame_length_samples));
			if (this->m_dither_coeff != 0.0 && sub.m_size > 0) {
				auto data = sub.m_data;
				for (size_t i = 0; i < sub.m_size; i++) {
					data[i] += dist(gen) * this->m_dither_coeff;
				}
			}
			if (this->m_subtract_mean) {
				auto sum = sub.Sum();
				sub.Add(-sum / sub.m_size);
			}
			if (this->m_preemphasis_coeff != 0.0) {
				auto data = sub.m_data;
				for (size_t i = sub.m_size - 1; i > 0; i--) {
					data[i] -= this->m_preemphasis_coeff * data[i - 1];
				}
				data[0] -= this->m_preemphasis_coeff * data[0];
			}
			sub.MulElements(this->m_window);
		}
		// Cache remaining samples in our temp buffer
		auto remain = data.m_size - (nframes * this->m_frame_shift_samples);
		this->field_x40.Resize(remain);
		if (remain > 0) {
			this->field_x40.CopyFromVec(data.Range(nframes * this->m_frame_shift_samples, remain));
		}
	}

	int FramerStream::NumFrames(int p1) const {
		if (this->m_frame_length_samples > p1) return 0;
		return ((p1 - this->m_frame_length_samples) / this->m_frame_shift_samples) + 1;
	}

	int FramerStream::Read(Matrix* mat, std::vector<FrameInfo>* info) {
		Matrix matrix_in;
		std::vector<FrameInfo> info_in;
		auto sig = m_connectedStream->Read(&matrix_in, &info_in);
		if ((sig & 0xc2) != 0 || matrix_in.m_cols == 0) {
			mat->Resize(0, 0);
			info->clear();
			return sig;
		}
		if (matrix_in.m_rows > 1) {
			SNOWBOY_WARNING() << "multiple channels detected for wave file; reading only the first channel";
		}

		Vector temp_vector;
		temp_vector.Resize(matrix_in.m_cols + field_x40.m_size);
		temp_vector.Range(0, field_x40.m_size).CopyFromVec(field_x40);
		temp_vector.Range(field_x40.m_size, matrix_in.m_cols).CopyFromVec(SubVector{matrix_in, 0});
		field_x40.Resize(0);

		CreateFrames(temp_vector, mat);

		info->resize(mat->m_rows);
		if (!info->empty()) {
			for (auto& e : *info) {
				e.frame_id = field_x38++;
			}
		}
		if ((sig & 0x18) != 0) {
			field_x40.Resize(0);
		}
		if ((sig & 0x10) != 0) {
			field_x38 = 1;
		}
		return sig;
	}

	bool FramerStream::Reset() {
		field_x40.Resize(0);
		return true;
	}

	std::string FramerStream::Name() const {
		return "FramerStream";
	}

	FramerStream::~FramerStream() {}
} // namespace snowboy
