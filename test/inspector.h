#pragma once

namespace snowboy {
	struct PipelinePersonalEnroll;
	class SnowboyPersonalEnroll;
	namespace testing {
		struct Inspector {
			static PipelinePersonalEnroll* SnowboyPersonalEnroll_GetEnrollPipeline(snowboy::SnowboyPersonalEnroll& enroll);
		};
	} // namespace testing
} // namespace snowboy
