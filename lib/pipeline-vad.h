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

	struct GainControlStreamOptions;
	struct FrontendStreamOptions;
	struct FramerStreamOptions;
	struct RawEnergyVadStreamOptions;
	struct VadStateStreamOptions;
	struct FftStreamOptions;
	struct MfccStreamOptions;
	struct RawNnetVadStreamOptions;

	struct PipelineVadOptions {
		int sampleRate;
		bool applyFrontend;

		void Register(const std::string& prefix, OptionsItf* opts);
	};
	struct PipelineVad : PipelineItf {
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
		PipelineVadOptions m_pipelineVadOptions;
		std::unique_ptr<GainControlStreamOptions> m_gainControlStreamOptions;
		std::unique_ptr<FrontendStreamOptions> m_frontendStreamOptions;
		std::unique_ptr<FramerStreamOptions> m_framerStreamOptions;
		std::unique_ptr<RawEnergyVadStreamOptions> m_rawEnergyVadStreamOptions;
		std::unique_ptr<VadStateStreamOptions> m_vadStateStreamOptions;
		std::unique_ptr<FftStreamOptions> m_fftStreamOptions;
		std::unique_ptr<MfccStreamOptions> m_mfccStreamOptions;
		std::unique_ptr<RawNnetVadStreamOptions> m_rawNnetVadStreamOptions;
		std::unique_ptr<VadStateStreamOptions> m_vadStateStream2Options;
		std::vector<FrameInfo> m_eavesdropStreamFrameInfoVector;
		bool field_xd0;
		bool field_xd1;

		virtual void RegisterOptions(const std::string& prefix, OptionsItf* opts) override;
		virtual int GetPipelineSampleRate() const override;
		virtual bool Init() override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual std::string OptionPrefix() const override;
		virtual ~PipelineVad();

		PipelineVad(const PipelineVadOptions& options);

		void ApplyFrontend(bool apply);
		int RunVad(const MatrixBase& data, bool is_end);
		void SetAudioGain(float gain);
		void SetMaxAudioAmplitude(float maxAmplitude);
	};
} // namespace snowboy
