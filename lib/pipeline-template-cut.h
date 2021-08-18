#pragma once
#include <framer-stream.h>
#include <matrix-wrapper.h>
#include <memory>
#include <pipeline-itf.h>

namespace snowboy {
	struct MatrixBase;
	struct FrameInfo;

	class InterceptStream;
	struct FramerStream;
	class EavesdropStream;
	class FftStream;
	class MfccStream;
	struct RawNnetVadStream;

	struct FramerStreamOptions;
	struct FftStreamOptions;
	struct MfccStreamOptions;
	struct RawNnetVadStreamOptions;

	struct PipelineTemplateCutOptions {
		int sample_rate;
		int min_non_voice_frames;
		int min_voice_frames;
		float bg_energy_threshold;

		void Register(const std::string& prefix, OptionsItf* opts);
	};
	class PipelineTemplateCut : public PipelineItf {
		std::unique_ptr<InterceptStream> m_interceptStream;
		std::unique_ptr<FramerStream> m_framerStream;
		std::unique_ptr<EavesdropStream> m_eavesdropStream;
		std::unique_ptr<FftStream> m_fftStream;
		std::unique_ptr<MfccStream> m_mfccStream;
		std::unique_ptr<RawNnetVadStream> m_rawNnetVadStream;

		PipelineTemplateCutOptions m_pipelineTemplateCutOptions;
		FramerStreamOptions m_framerStreamOptions;
		std::unique_ptr<FftStreamOptions> m_fftStreamOptions;
		std::unique_ptr<MfccStreamOptions> m_mfccStreamOptions;
		std::unique_ptr<RawNnetVadStreamOptions> m_rawNnetVadStreamOptions;
		Matrix m_eavesdropMatrix;

	public:
		virtual void RegisterOptions(const std::string& prefix, OptionsItf* opts) override;
		virtual int GetPipelineSampleRate() const override;
		virtual bool Init() override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual std::string OptionPrefix() const override;
		virtual ~PipelineTemplateCut();

		PipelineTemplateCut(const PipelineTemplateCutOptions& options);

		int CutTemplate(const MatrixBase& in, Matrix* out);
		void ComputeTemplateBoundary(const MatrixBase&, const std::vector<FrameInfo>&, int*, int*) const;
	};
} // namespace snowboy
