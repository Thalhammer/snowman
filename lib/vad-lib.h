#pragma once
#include <cstdint>
#include <vector>

namespace snowboy {
	struct VadStateOptions {
		uint32_t min_non_voice_frames;
		uint32_t min_voice_frames;
	};
	enum VoiceType { VT_0,
					 VT_1,
					 VT_2 };
	enum VoiceStateType { VST_0,
						  VST_1,
						  VST_2 };
	struct VadState {
		VadStateOptions m_options;
		bool m_field_x10;
		size_t m_field_x14;

		VadState(VadStateOptions options);
		virtual ~VadState();
		void Reset();
		void GetVoiceStates(const std::vector<VoiceType>&, std::vector<VoiceStateType>*);
	};
} // namespace snowboy
