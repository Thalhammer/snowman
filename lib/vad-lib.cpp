#include <vad-lib.h>

namespace snowboy {
	VadState::VadState(VadStateOptions options) {
		m_options = options;
		Reset();
	}

	VadState::~VadState() {}

	void VadState::Reset() {
		m_field_x10 = 0;
		m_field_x14 = 0;
	}

	void VadState::GetVoiceStates(const std::vector<VoiceType>& param_1, std::vector<VoiceStateType>* param_2) {
		param_2->resize(param_1.size(), VST_0);
		if (param_1.size() == 0) return;
		auto pVVar3 = param_2->begin();
		auto piVar9 = param_1.begin();
		while (piVar9 != param_1.end()) {
			auto VVar1 = *piVar9;
			if (m_field_x10 != 0) {
				if (VVar1 == 1) {
					m_field_x14 = 0;
					*pVVar3 = VST_1;
				} else if (VVar1 == 2) {
					if (m_field_x14 < m_options.min_non_voice_frames) {
						m_field_x14++;
						*pVVar3 = VST_1;
					} else {
						m_field_x14 = 0;
						m_field_x10 = false;
						*pVVar3 = VST_2;
					}
				}
			} else {
				if (VVar1 == 1) {
					if (m_field_x14 < m_options.min_voice_frames) {
						m_field_x14++;
						*pVVar3 = VST_2;
					} else {
						m_field_x10 = true;
						m_field_x14 = 0;
						*pVVar3 = VST_1;
					}
				} else if (VVar1 == 2) {
					m_field_x14 = 0;
					*pVVar3 = VST_2;
				}
			}
			piVar9++;
			pVVar3++;
		}
	}

} // namespace snowboy
