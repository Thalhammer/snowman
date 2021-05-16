#include <cassert>
#include <limits>
#include <snowboy-debug.h>
#include <snowboy-io.h>
#include <template-container.h>
#include <vector-wrapper.h>

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

	void TemplateContainer::CombineTemplates(DistanceType distance) {
		if (m_templates.size() < 2) return;
		auto fVar1 = std::numeric_limits<float>::max();
		auto local_88 = 0;
		for (auto uVar13 = 0; uVar13 < m_templates.size(); uVar13++) {
			auto sum = 0.0;
			for (auto uVar19 = 0; uVar19 < m_templates.size(); uVar19++) {
				if (uVar13 != uVar19) {
					sum += snowboy::DtwAlign(distance, m_templates[uVar13], m_templates[uVar19], nullptr);
				}
			}
			if (sum < fVar1) {
				local_88 = uVar13;
				fVar1 = sum;
			}
		}
		std::vector<int> local_a0;
		local_a0.resize(m_templates[local_88].m_rows, 1); // Not sure if int

		if (m_templates.size() != 0) {
			auto local_90 = 0;
			do {
				if (local_88 != (int)local_90) {
					std::vector<std::vector<int>> local_58;
					snowboy::DtwAlign(distance, m_templates[local_88], m_templates[local_90], &local_58);
					for (auto local_a8 = 0; local_a8 < m_templates[local_88].m_rows; local_a8 += 1) {
						if (local_58[local_a8].size() != 0) {
							SubVector{m_templates[local_88], local_a8}.Scale(local_a0[local_a8]);
							for (auto uVar10 = 0; uVar10 < local_58[local_a8].size(); uVar10++) {
								SubVector{m_templates[local_88], local_a8}.AddVec(1.0, SubVector{m_templates[local_90], local_58[local_a8][uVar10]});
							}
							local_a0[local_a8] += local_58[local_a8].size();
							SubVector{m_templates[local_88], local_a8}.Scale(1.0f / (float)(local_a0[local_a8]));
						}
					}
				}
				local_90 += 1;
			} while (local_90 < m_templates.size());
		}
		if (local_88 != 0) {
			m_templates[0] = m_templates[local_88];
		}
		m_templates.resize(1);
	}

	void TemplateContainer::Clear() {
		m_templates.clear();
	}

	void TemplateContainer::AddTemplate(const MatrixBase& tpl) {
		m_templates.emplace_back(tpl);
	}

} // namespace snowboy