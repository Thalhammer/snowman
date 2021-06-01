#pragma once

namespace snowboy {
	enum SnowboySignal {};
	struct FrameInfo {
		// TODO: Should be size_t
		unsigned int frame_id = 0;
		int flags = 0;
	};
} // namespace snowboy
