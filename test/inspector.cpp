#include "inspector.h"
#include <snowboy-detect.h>

namespace snowboy {
	namespace testing {
		PipelinePersonalEnroll* Inspector::SnowboyPersonalEnroll_GetEnrollPipeline(SnowboyPersonalEnroll& enroll) {
			return enroll.enroll_pipeline_.get();
		}
	} // namespace testing
} // namespace snowboy
