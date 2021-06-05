#include "inspector.h"
#include <snowboy-detect.h>
#include <pipeline-personal-enroll.h>

namespace snowboy {
	namespace testing {
		PipelinePersonalEnroll* Inspector::SnowboyPersonalEnroll_GetEnrollPipeline(SnowboyPersonalEnroll& enroll) {
			return enroll.enroll_pipeline_.get();
		}
		TemplateEnrollStream* Inspector::PipelinePersonalEnroll_GetTemplateEnrollStream(snowboy::PipelinePersonalEnroll* enroll) {
			return enroll->m_templateEnrollStream.get();
		}
	} // namespace testing
} // namespace snowboy
