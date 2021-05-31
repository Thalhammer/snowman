#pragma once
#include <matrix-wrapper.h>
#include <stream-itf.h>
#include <string>
#include <template-container.h>

namespace snowboy {
	struct OptionsItf;

	struct TemplateEnrollStreamOptions {
		uint32_t num_templates;
		uint32_t min_template_length;
		uint32_t max_template_length;
		std::string combine_distance_metric;
		std::string model_filename;
		void Register(const std::string&, OptionsItf*);
	};
	struct TemplateEnrollStream : StreamItf {
		TemplateEnrollStreamOptions m_options;
		TemplateContainer field_x38;
		Matrix field_x60;
		int field_x78;

		TemplateEnrollStream(const TemplateEnrollStreamOptions& options);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~TemplateEnrollStream();

		void SetModelFilename(const std::string& name);
	};
} // namespace snowboy
