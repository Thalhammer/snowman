#pragma once
#include <matrix-wrapper.h>
#include <stream-itf.h>
#include <string>
#include <template-container.h>

namespace snowboy {
	struct OptionsItf;

	struct TemplateEnrollStreamOptions {
		int num_templates;
		int min_template_length;
		int max_template_length;
		std::string combine_distance_metric;
		std::string model_filename;
		void Register(const std::string&, OptionsItf*);
	};
	static_assert(sizeof(TemplateEnrollStreamOptions) == 0x20);
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
	static_assert(sizeof(TemplateEnrollStream) == 0x80);
} // namespace snowboy