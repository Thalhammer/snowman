#pragma once
#include <stream-itf.h>

namespace snowboy {
	struct OptionsItf;
	struct GainControlStreamOptions {
		float m_audioGain;
		void Register(const std::string&, OptionsItf*);
	};
	class GainControlStream : public StreamItf {
		float m_audioGain;
		float m_maxAudioAmplitude;

	public:
		GainControlStream(const GainControlStreamOptions& options);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~GainControlStream();

		void SetAudioGain(float gain);
		void SetMaxAudioAmplitude(float amp);
	};
} // namespace snowboy
