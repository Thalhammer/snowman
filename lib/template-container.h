#pragma once
#include <matrix-wrapper.h>
#include <dtw-lib.h>

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
        const Matrix* GetTemplate(int index) const;
        void DeleteTemplate(int index);
        void CombineTemplates(DistanceType distance);
        void Clear();
        void AddTemplate(const MatrixBase& tpl);
    };
    static_assert(sizeof(TemplateContainer) == 0x28);
} // namespace snowboy