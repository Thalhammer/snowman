#include <frame-info.h>
#include <snowboy-error.h>
#include <snowboy-options.h>
#include <template-enroll-stream.h>

namespace snowboy {
	void TemplateEnrollStreamOptions::Register(const std::string& prefix, OptionsItf* opts) {
		opts->Register(prefix, "num-templates", "Number of templates to be enrolled", &num_templates);
		opts->Register(prefix, "min-template-length", "Minimal required length of template.", &min_template_length);
		opts->Register(prefix, "max-template-length", "Maximal possible length of template.", &max_template_length);
		opts->Register(prefix, "combine-distance-metric", "If not empty, combines all templates into one template using dynamic time warping with the specified distance type.", &combine_distance_metric);
		opts->Register(prefix, "model-filename", "File that we want to write the templates to.", &model_filename);
	}

	TemplateEnrollStream::TemplateEnrollStream(const TemplateEnrollStreamOptions& options)
		: m_options{options}, field_x38{}, field_x60{}, field_x78{0} {}

	int TemplateEnrollStream::Read(Matrix* mat, std::vector<FrameInfo>* info) {
		if (mat) mat->Resize(0, 0);
		if (info) info->clear();
		if (m_options.num_templates <= field_x38.NumTemplates()) return 0x10;
		Matrix local_1f8;
		std::vector<FrameInfo> local_1d8;
		auto res = m_connectedStream->Read(&local_1f8, &local_1d8);
		if ((res & 0xc2) != 0) return res;
		if (local_1f8.m_rows > 0) {
			field_x60.Resize(local_1f8.m_rows + field_x60.m_rows, local_1f8.m_cols, MatrixResizeType::kCopyData);
			field_x60.RowRange(field_x60.m_rows - local_1f8.m_rows, local_1f8.m_rows).CopyFromMat(local_1f8, MatrixTransposeType::kNoTrans);
		}
		if ((res & 0x18) != 0) {
			if (field_x60.rows() < m_options.min_template_length) {
				field_x60.Resize(0, 0);
				res |= 0x400;
				return res;
			}
			if (field_x60.rows() > m_options.max_template_length) {
				field_x60.Resize(0, 0);
				res |= 0x200;
				return res;
			}
			field_x38.AddTemplate(field_x60);
			field_x60.Resize(0, 0);
			field_x78++;
		}
		if (field_x38.NumTemplates() == m_options.num_templates) {
			if (m_options.combine_distance_metric == "cosine") {
				field_x38.CombineTemplates(DistanceType::cosine);
			} else if (m_options.combine_distance_metric == "euclidean") {
				field_x38.CombineTemplates(DistanceType::euclidean);
			} else if (m_options.combine_distance_metric != "") {
				throw snowboy_exception{"unknown distance metric \"" + m_options.combine_distance_metric + "\""};
				return -1;
			}
			field_x38.WriteHotwordModel(true, m_options.model_filename);
			res |= 0x10;
		}
		return res;
	}

	bool TemplateEnrollStream::Reset() {
		field_x78 = 0;
		field_x60.Resize(0, 0);
		field_x38.Clear();
		return true;
	}

	std::string TemplateEnrollStream::Name() const {
		return "TemplateEnrollStream";
	}

	TemplateEnrollStream::~TemplateEnrollStream() {}

	void TemplateEnrollStream::SetModelFilename(const std::string& name) {
		m_options.model_filename = name;
	}
} // namespace snowboy
