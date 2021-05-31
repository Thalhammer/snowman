#pragma once
#include <dtw-lib.h>
#include <matrix-wrapper.h>

namespace snowboy {
	struct TemplateContainer {
		float m_sensitivity;
		std::vector<Matrix> m_templates;

		TemplateContainer();
		TemplateContainer(float sensitivity);
		virtual ~TemplateContainer();
		void WriteHotwordModel(bool binary, const std::string& filename) const;
		void ReadHotwordModel(const std::string& filename);
		size_t NumTemplates() const;
		const Matrix* GetTemplate(size_t index) const;
		void DeleteTemplate(size_t index);
		void CombineTemplates(DistanceType distance);
		void Clear();
		void AddTemplate(const MatrixBase& tpl);
	};
} // namespace snowboy
