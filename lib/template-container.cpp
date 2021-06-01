#include <cassert>
#include <limits>
#include <snowboy-error.h>
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
		WriteBasicType<int32_t>(binary, m_templates.size(), os);
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
		ReadBasicType<int32_t>(binary, &num, is);
		m_templates.resize(num);
		for (auto& e : m_templates) {
			ExpectToken(binary, "<Template>", is);
			e.Read(binary, is);
		}
	}

	size_t TemplateContainer::NumTemplates() const {
		return m_templates.size();
	}

	const Matrix* TemplateContainer::GetTemplate(size_t index) const {
		if (index >= m_templates.size())
			throw snowboy_exception{"template id runs out of range, expecting a value between [0, "
									+ std::to_string(m_templates.size()) + "] got " + std::to_string(index) + " instead."};
		return &m_templates[index];
	}

	void TemplateContainer::DeleteTemplate(size_t index) {
		if (index >= m_templates.size())
			throw snowboy_exception{"template id runs out of range, expecting a value between [0, "
									+ std::to_string(m_templates.size()) + "] got " + std::to_string(index) + " instead."};
		m_templates.erase(m_templates.begin() + index);
	}

	void TemplateContainer::CombineTemplates(DistanceType distance) {
		if (m_templates.size() < 2) return;
		auto min_val = std::numeric_limits<float>::max();
		size_t min_idx = 0;
		for (size_t i = 0; i < m_templates.size(); i++) {
			auto sum = 0.0;
			for (size_t i2 = 0; i2 < m_templates.size(); i2++) {
				if (i != i2) {
					sum += snowboy::DtwAlign(distance, m_templates[i], m_templates[i2], nullptr);
				}
			}
			if (sum < min_val) {
				min_idx = i;
				min_val = sum;
			}
		}
		std::vector<int> local_a0;
		local_a0.resize(m_templates[min_idx].m_rows, 1); // Not sure if int

		for (size_t local_90 = 0; local_90 < m_templates.size(); local_90++) {
			if (min_idx != local_90) {
				std::vector<std::vector<size_t>> local_58;
				snowboy::DtwAlign(distance, m_templates[min_idx], m_templates[local_90], &local_58);
				for (size_t local_a8 = 0; local_a8 < m_templates[min_idx].rows(); local_a8 += 1) {
					if (local_58[local_a8].size() != 0) {
						SubVector{m_templates[min_idx], local_a8}.Scale(local_a0[local_a8]);
						for (size_t uVar10 = 0; uVar10 < local_58[local_a8].size(); uVar10++) {
							SubVector{m_templates[min_idx], local_a8}.AddVec(1.0, SubVector{m_templates[local_90], local_58[local_a8][uVar10]});
						}
						local_a0[local_a8] += local_58[local_a8].size();
						SubVector{m_templates[min_idx], local_a8}.Scale(1.0f / (float)(local_a0[local_a8]));
					}
				}
			}
		}
		if (min_idx != 0) {
			m_templates[0] = m_templates[min_idx];
		}
		m_templates.resize(1);
	}

	void TemplateContainer::Clear() {
		m_templates.clear();
	}

	void TemplateContainer::AddTemplate(const MatrixBase& tpl) {
		SNOWBOY_ASSERT(!tpl.HasNan() && !tpl.HasInfinity());
		m_templates.emplace_back(tpl);
	}

} // namespace snowboy
