#pragma once
#include <memory>
#include <string>
#include <stream-itf.h>
#include <deque>
#include <matrix-wrapper.h>
#include <template-container.h>
#include <dtw-lib.h>

namespace snowboy {
	struct OptionsItf;

	struct TemplateDetectStreamOptions {
		int slide_step;
        std::string sensitivity_str;
        std::string model_str;
        SlidingDtwOptions dtw_options;
		void Register(const std::string&, OptionsItf*);
	};
	static_assert(sizeof(TemplateDetectStreamOptions) == 0x28);
	struct TemplateDetectStream : StreamItf {
        TemplateDetectStreamOptions m_options;
        std::vector<TemplateContainer> field_x40;
        std::vector<std::vector<SlidingDtw>> field_x58;
        int field_x70;
        Matrix field_x78;
        int field_x90;

		TemplateDetectStream(const TemplateDetectStreamOptions& options);
		virtual int Read(Matrix* mat, std::vector<FrameInfo>* info) override;
		virtual bool Reset() override;
		virtual std::string Name() const override;
		virtual ~TemplateDetectStream();

        void SetSensitivity(const std::string& sensitivities);
        std::string GetSensitivity() const;
        void InitDtw();
        size_t NumHotwords(int model_id) const;
        void UpdateModel() const;
	};
	static_assert(sizeof(TemplateDetectStream) == 0x98);
} // namespace snowboy