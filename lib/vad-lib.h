#pragma once
#include <vector>

namespace snowboy {
	struct VadStateOptions {
		int min_non_voice_frames;
		int min_voice_frames;
	};
	static_assert(sizeof(VadStateOptions) == 8);
	enum VoiceType { VT_0,
					 VT_1,
					 VT_2 };
	enum VoiceStateType { VST_0,
						  VST_1,
						  VST_2 };
	struct VadState {
		VadStateOptions m_options;
		bool m_field_x10;
		int m_field_x14;

		VadState(VadStateOptions options);
		virtual ~VadState();
		void Reset();
		void GetVoiceStates(const std::vector<VoiceType>&, std::vector<VoiceStateType>*);
	};
	static_assert(sizeof(VadState) == 24);
} // namespace snowboy