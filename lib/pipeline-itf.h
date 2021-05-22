#pragma once
#include <string>
namespace snowboy {
	struct OptionsItf;
	class PipelineItf {
	public:
		virtual void RegisterOptions(const std::string&, OptionsItf*) = 0;
		virtual void SetResource(const std::string&);
		virtual int GetPipelineSampleRate() const = 0;
		virtual bool Init() = 0;
		virtual bool Reset() = 0;
		virtual std::string Name() const = 0;
		virtual std::string OptionPrefix() const = 0;
		virtual ~PipelineItf();

	protected:
		bool m_isInitialized = false;
	};
} // namespace snowboy
