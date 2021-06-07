#pragma once
#include <memory>
#include <pipeline-itf.h>
#include <vector>

namespace snowboy {
	struct MatrixBase;
	struct FrameInfo;

	class InterceptStream;
	class GainControlStream;
	struct FrontendStream;
	struct FramerStream;
	struct RawEnergyVadStream;
	struct VadStateStream;
	class FftStream;
	class MfccStream;
	struct RawNnetVadStream;
	class EavesdropStream;
	class NnetStream;
	struct TemplateDetectStream;
	struct UniversalDetectStream;

	struct GainControlStreamOptions;
	struct FrontendStreamOptions;
	struct FramerStreamOptions;
	struct RawEnergyVadStreamOptions;
	struct VadStateStreamOptions;
	struct FftStreamOptions;
	struct MfccStreamOptions;
	struct RawNnetVadStreamOptions;
	struct NnetStreamOptions;
	struct TemplateDetectStreamOptions;
	struct UniversalDetectStreamOptions;

	struct PipelineDetectOptions {
		int sampleRate;
		bool applyFrontend;
		// Padding
		void Register(const std::string&, OptionsItf*);
	};

	class PipelineDetect : public PipelineItf {
	public:
		virtual void RegisterOptions(const std::string&, OptionsItf*) override;
		virtual int GetPipelineSampleRate() const override;
		virtual bool Init() override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual std::string OptionPrefix() const override;
		virtual ~PipelineDetect();

		PipelineDetect(const PipelineDetectOptions& options);

		void ApplyFrontend(bool apply);
		uint64_t GetDetectedFrameId() const;
		std::string GetSensitivity() const;
		int NumHotwords() const;
		int RunDetection(const MatrixBase& data, bool is_end);
		void SetAudioGain(float gain);
		void SetHighSensitivity(const std::string&);
		void SetMaxAudioAmplitude(float maxAmplitude);
		void SetModel(const std::string& model);
		void SetSensitivity(const std::string& sensitivity);
		void UpdateModel() const;

	private:
		void ClassifyModels(const std::string&, std::string*, std::string*);
		bool ClassifyModel(const std::string& model_filename);
		void ClassifySensitivities(const std::string&, std::string*, std::string*) const;

		std::unique_ptr<InterceptStream> m_interceptStream;
		std::unique_ptr<GainControlStream> m_gainControlStream;
		std::unique_ptr<FrontendStream> m_frontendStream;
		std::unique_ptr<FramerStream> m_framerStream;
		std::unique_ptr<RawEnergyVadStream> m_rawEnergyVadStream;
		std::unique_ptr<VadStateStream> m_vadStateStream;
		std::unique_ptr<FftStream> m_fftStream;
		std::unique_ptr<MfccStream> m_mfccStream;
		std::unique_ptr<RawNnetVadStream> m_rawNnetVadStream;
		std::unique_ptr<VadStateStream> m_vadStateStream2;
		std::unique_ptr<EavesdropStream> m_eavesdropStream;

		std::unique_ptr<InterceptStream> m_templateDetectInterceptStream;
		std::unique_ptr<NnetStream> m_templateDetectNnetStream;
		std::unique_ptr<TemplateDetectStream> m_templateDetectStream;

		std::unique_ptr<InterceptStream> m_universalDetectInterceptStream;
		std::unique_ptr<UniversalDetectStream> m_universalDetectStream;

		PipelineDetectOptions m_pipelineDetectOptions = {};
		std::unique_ptr<GainControlStreamOptions> m_gainControlStreamOptions;
		std::unique_ptr<FrontendStreamOptions> m_frontendStreamOptions;
		std::unique_ptr<FramerStreamOptions> m_framerStreamOptions;
		std::unique_ptr<RawEnergyVadStreamOptions> m_rawEnergyVadStreamOptions;
		std::unique_ptr<VadStateStreamOptions> m_vadStateStreamOptions;
		std::unique_ptr<FftStreamOptions> m_fftStreamOptions;
		std::unique_ptr<MfccStreamOptions> m_mfccStreamOptions;
		std::unique_ptr<RawNnetVadStreamOptions> m_rawNnetVadStreamOptions;
		std::unique_ptr<VadStateStreamOptions> m_vadStateStream2Options;
		std::unique_ptr<NnetStreamOptions> m_templateDetectNnetStreamOptions;
		std::unique_ptr<TemplateDetectStreamOptions> m_templateDetectStreamOptions;
		std::unique_ptr<UniversalDetectStreamOptions> m_universalDetectStreamOptions;

		std::vector<FrameInfo> m_eavesdropStreamFrameInfoVector;
		std::vector<bool> m_is_personal_model;
		std::vector<int> m_personal_kw_mapping;
		std::vector<int> m_universal_kw_mapping;

		bool field_x168 = false;
		bool m_frontend_enabled = false;
	};
} // namespace snowboy
