#pragma once

namespace snowboy {
	struct PipelinePersonalEnroll;
	class TemplateEnrollStream;
	class SnowboyPersonalEnroll;
	namespace testing {
		struct Inspector {
			static PipelinePersonalEnroll* SnowboyPersonalEnroll_GetEnrollPipeline(snowboy::SnowboyPersonalEnroll& enroll);
			static TemplateEnrollStream* PipelinePersonalEnroll_GetTemplateEnrollStream(snowboy::PipelinePersonalEnroll* enroll);
		};
	} // namespace testing
} // namespace snowboy
