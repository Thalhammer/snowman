#include <snowboy-debug.h>
#include <snowboy-io.h>
#include <template-container.h>

namespace snowboy {
	TemplateContainer::TemplateContainer() {
		m_sensitivity = 0.4f;
	}

	TemplateContainer::TemplateContainer(float sensitivity) {
		m_sensitivity = sensitivity;
	}

	TemplateContainer::~TemplateContainer() {
	}

	void TemplateContainer::WriteHotwordModel(bool binary, const std::string& filename) const {
		Output out{filename, binary};
		auto os = out.Stream();
		WriteToken(binary, "<PersonalModel>", os);
		WriteToken(binary, "<Sensitivity>", os);
		WriteBasicType<float>(binary, m_sensitivity, os);
		WriteToken(binary, "<NumTemplates>", os);
		WriteBasicType<int>(binary, m_templates.size(), os);
		for (auto& e : m_templates) {
			WriteToken(binary, "<Template>", os);
			e.Write(binary, os);
		}
	}

	void TemplateContainer::ReadHotwordModel(const std::string& filename) {
		Input in{filename};
		auto is = in.Stream();
		auto binary = in.is_binary();

		ExpectToken(binary, "<PersonalModel>", is);
		ExpectToken(binary, "<Sensitivity>", is);
		ReadBasicType<float>(binary, &m_sensitivity, is);
		ExpectToken(binary, "<NumTemplates>", is);
		int num = -1;
		ReadBasicType<int>(binary, &num, is);
		m_templates.resize(num);
		for (auto& e : m_templates) {
			ExpectToken(binary, "<Template>", is);
			e.Read(binary, is);
		}
	}

	size_t TemplateContainer::NumTemplates() const {
		return m_templates.size();
	}

	const Matrix* TemplateContainer::GetTemplate(int index) const {
		if (index < 0 || index >= m_templates.size()) {
			SNOWBOY_ERROR() << "template id runs out of range, expecting a value between [0, " << m_templates.size() << "] got " << index << " instead.";
			return nullptr;
		}
		return &m_templates[index];
	}

	void TemplateContainer::DeleteTemplate(int index) {
		if (index < 0 || index >= m_templates.size()) {
			SNOWBOY_ERROR() << "template id runs out of range, expecting a value between [0, " << m_templates.size() << "] got " << index << " instead.";
			return;
		}
		m_templates.erase(m_templates.begin() + index);
	}

	// TODO: I haven't found time to reverse this yet, but its not referenced by the detection
	// TODO: part of snowboy anyway.
	//void TemplateContainer::CombineTemplates(DistanceType distance) {
	//    // TODO
	//}

	void TemplateContainer::Clear() {
		m_templates.clear();
	}

	void TemplateContainer::AddTemplate(const MatrixBase& tpl) {
		m_templates.emplace_back(tpl);
	}

} // namespace snowboy