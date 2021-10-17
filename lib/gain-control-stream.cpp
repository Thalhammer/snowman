#include <gain-control-stream.h>
#include <matrix-wrapper.h>
#include <snowboy-error.h>
#include <snowboy-options.h>

namespace snowboy {
	bool StreamItf::Connect(StreamItf* other) {
		m_connectedStream = other;
		m_isConnected = true;
		return true;
	}

	bool StreamItf::Disconnect() {
		m_isConnected = false;
		m_connectedStream = nullptr;
		return true;
	}

	StreamItf::~StreamItf() {}

	void GainControlStreamOptions::Register(const std::string& prefix, OptionsItf* opts) {
		opts->Register(prefix, "audio-gain", "Gain to be applied to raw input audio", &m_audioGain);
	}

	GainControlStream::GainControlStream(const GainControlStreamOptions& options) {
		m_connectedStream = nullptr;
		m_isConnected = false;
		SetAudioGain(options.m_audioGain);
		m_maxAudioAmplitude = 32767.0;
	}

	int GainControlStream::Read(Matrix* mat, std::vector<FrameInfo>* info) {
		auto res = m_connectedStream->Read(mat, info);
		if ((res & 0xc2) == 0 && m_audioGain != 1.0 && mat->m_rows > 0) {
			auto len = mat->m_rows * mat->m_stride;
			auto ptr = mat->m_data;
			for (size_t i = 0; i < len; i++) {
				auto v = ptr[i];
				v /= m_maxAudioAmplitude;
				v *= m_audioGain;
				if (v >= 1.0)
					v = 1.0;
				else if (v <= -1.0)
					v = -1.0;
				else
					v = v * 1.5 - v * v * 0.5 * v;
				ptr[i] = v * m_maxAudioAmplitude;
			}
		}
		return res;
	}

	bool GainControlStream::Reset() {
		return true;
	}

	std::string GainControlStream::Name() const {
		return "GainControlStream";
	}

	GainControlStream::~GainControlStream() {}

	void GainControlStream::SetAudioGain(float gain) {
		if (gain <= 0.0) {
			throw snowboy_exception{"audio gain must be non-negative"};
		}
		m_audioGain = gain;
	}

	void GainControlStream::SetMaxAudioAmplitude(float amp) {
		if (amp <= 0.0) {
			throw snowboy_exception{"max audio amplitude must be non-negative"};
		}
		m_maxAudioAmplitude = amp;
	}

} // namespace snowboy
