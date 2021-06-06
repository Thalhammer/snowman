#pragma once
#include <deque>
#include <dtw-lib.h>
#include <matrix-wrapper.h>
#include <memory>
#include <stream-itf.h>
#include <string>
#include <template-container.h>

namespace snowboy {
	struct OptionsItf;

	struct TemplateDetectStreamOptions {
		int slide_step;
		std::string sensitivity_str;
		std::string model_str;
		SlidingDtwOptions dtw_options;
		void Register(const std::string&, OptionsItf*);
	};
	struct TemplateDetectStream : StreamItf {
		TemplateDetectStreamOptions m_options;
		std::vector<TemplateContainer> m_models;
		std::vector<std::vector<SlidingDtw>> field_x58;
		size_t field_x70;
		Matrix field_x78;
		int field_x90;
		void InitDtw();

		TemplateDetectStream(const TemplateDetectStreamOptions& options);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~TemplateDetectStream();

		void SetSensitivity(const std::string& sensitivities);
		std::string GetSensitivity() const;
		size_t NumHotwords(size_t model_id) const;
		void UpdateModel() const;
	};
} // namespace snowboy
