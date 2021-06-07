#pragma once
#include <memory>
#include <pipeline-itf.h>
#include <vector>

namespace snowboy {
	namespace testing {
		class Inspector;
	}
	struct MatrixBase;
	struct FrameInfo;

	class InterceptStream;
	struct FramerStream;
	class FftStream;
	class MfccStream;
	class NnetStream;
	struct TemplateEnrollStream;

	struct FramerStreamOptions;
	struct FftStreamOptions;
	struct MfccStreamOptions;
	struct NnetStreamOptions;
	struct TemplateEnrollStreamOptions;

	struct PipelinePersonalEnrollOptions {
		int sample_rate;

		void Register(const std::string& prefix, OptionsItf* opts);
	};
	class PipelinePersonalEnroll : public PipelineItf {
		friend class testing::Inspector;
		std::unique_ptr<InterceptStream> m_interceptStream;
		std::unique_ptr<FramerStream> m_framerStream;
		std::unique_ptr<FftStream> m_fftStream;
		std::unique_ptr<MfccStream> m_mfccStream;
		std::unique_ptr<NnetStream> m_nnetStream;
		std::unique_ptr<TemplateEnrollStream> m_templateEnrollStream;

		PipelinePersonalEnrollOptions m_pipelinePersonalEnrollOptions;
		std::unique_ptr<FramerStreamOptions> m_framerStreamOptions;
		std::unique_ptr<FftStreamOptions> m_fftStreamOptions;
		std::unique_ptr<MfccStreamOptions> m_mfccStreamOptions;
		std::unique_ptr<NnetStreamOptions> m_nnetStreamOptions;
		std::unique_ptr<TemplateEnrollStreamOptions> m_templateEnrollStreamOptions;

	public:
		virtual void RegisterOptions(const std::string& prefix, OptionsItf* opts) override;
		virtual int GetPipelineSampleRate() const override;
		virtual bool Init() override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual std::string OptionPrefix() const override;
		virtual ~PipelinePersonalEnroll();

		PipelinePersonalEnroll(const PipelinePersonalEnrollOptions& options);

		int RunEnrollment(const MatrixBase& data);
		void SetModelFilename(const std::string& filename);
		int GetNumTemplates() const;
	};
} // namespace snowboy
