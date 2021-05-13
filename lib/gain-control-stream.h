#pragma once
#include <stream-itf.h>

namespace snowboy {
	struct OptionsItf;
	struct GainControlStreamOptions {
		float m_audioGain;
		void Register(const std::string&, OptionsItf*);
	};
	static_assert(sizeof(GainControlStreamOptions) == 4);
	struct GainControlStream : StreamItf {
		float m_audioGain;
		float m_maxAudioAmplitude;

		GainControlStream(const GainControlStreamOptions& options);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~GainControlStream();

		void SetAudioGain(float gain);
		void SetMaxAudioAmplitude(float amp);
	};
	static_assert(sizeof(GainControlStream) == 0x20);
} // namespace snowboy