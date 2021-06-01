#include <frame-info.h>
#include <limits>
#include <nnet-lib.h>
#include <snowboy-error.h>
#include <snowboy-io.h>
#include <snowboy-options.h>
#include <template-detect-stream.h>

namespace snowboy {
	void TemplateDetectStreamOptions::Register(const std::string& prefix, OptionsItf* opts) {
		opts->Register(prefix, "band-width", "Band width for segmental DTW.", &dtw_options.band_width);
		opts->Register(prefix, "distance-metric", "Distance metric for DTW, candidates are: cosine|euclidean.", &dtw_options.distance_metric);
		opts->Register(prefix, "slide-step", "Step size for sliding window in frames.", &slide_step);
		opts->Register(prefix, "sensitivity-str", "String that contains the sensitivity for each hotword, separated by comma.", &sensitivity_str);
		opts->Register(prefix, "model-str", "String that contains hotword models, separated by comma.", &model_str);
	}

	TemplateDetectStream::TemplateDetectStream(const TemplateDetectStreamOptions& options) {
		m_options = options;
		if (m_options.model_str == "")
			throw snowboy_exception{"please specify models through --model-str"};
		if (m_options.slide_step < 1)
			throw snowboy_exception{"slide step size should be positive"};
		field_x70 = 0;
		field_x78.Resize(0, 0);
		field_x90 = -100;
		std::vector<std::string> models;
		SplitStringToVector(m_options.model_str, ",", &models);
		if (models.empty())
			throw snowboy_exception{"no model can be extracted from --model-str:" + m_options.model_str};
		m_models.resize(models.size());
		for (size_t i = 0; i < models.size(); i++) {
			m_models[i].ReadHotwordModel(models[i]);
		}
		InitDtw();
		if (m_options.sensitivity_str != "") {
			SetSensitivity(m_options.sensitivity_str);
		}
	}

	int TemplateDetectStream::Read(Matrix* mat, std::vector<FrameInfo>* info) {
		mat->Resize(0, 0);
		info->clear();
		Matrix read_mat;
		std::vector<FrameInfo> read_info;
		auto read_res = m_connectedStream->Read(&read_mat, &read_info);
		if ((read_res & 0xc2) == 0 && read_mat.m_rows != 0) {
			auto old_f78_size = field_x78.m_rows;
			field_x78.Resize(read_mat.m_rows + old_f78_size, read_mat.m_cols, MatrixResizeType::kCopyData);
			field_x78.RowRange(old_f78_size, read_mat.m_rows).CopyFromMat(read_mat, MatrixTransposeType::kNoTrans);

			for (size_t slide_pos = 0; slide_pos < read_mat.rows(); slide_pos += m_options.slide_step) {
				for (size_t model_id = 0; model_id < field_x58.size(); model_id++) {
					auto matched_templates = 0;
					for (size_t template_id = 0; template_id < field_x58[model_id].size(); template_id++) {
						auto step = m_options.slide_step;
						if (read_mat.m_rows < slide_pos + step) step = read_mat.m_rows - slide_pos;
						auto iVar2 = step - 1 + old_f78_size + slide_pos;
						int window_size = field_x58[model_id][template_id].GetWindowSize();
						window_size = (iVar2 - window_size) + 1;
						if (window_size < 0) window_size = 0;
						auto distance = field_x58[model_id][template_id].ComputeDtwDistance(step, field_x78.RowRange(window_size, (iVar2 - window_size) + 1));
						if (distance < m_models[model_id].m_sensitivity) matched_templates++;
					}
					if (field_x58[model_id].size() * 0.5f < matched_templates) {
						mat->Resize(1, 1, MatrixResizeType::kSetZero);
						mat->m_data[0] = model_id + 1;
						info->resize(1);
						info->at(0) = read_info[slide_pos];
						field_x90 = read_info[slide_pos].frame_id;
						this->Reset();
						return read_res;
					}
				}
			}
		}
		if (field_x70 < field_x78.rows()) {
			auto x = field_x78.RowRange(field_x78.m_rows - field_x70, field_x70);
			Matrix m;
			m.Resize(x.m_rows, x.m_cols, MatrixResizeType::kUndefined);
			m.CopyFromMat(x, MatrixTransposeType::kNoTrans);
			field_x78 = std::move(m);
		}
		if ((read_res & 0x18) != 0) {
			this->Reset();
		}
		return read_res;
	}

	bool TemplateDetectStream::Reset() {
		for (auto& m : field_x58) {
			for (auto& t : m)
				t.Reset();
		}
		field_x78.Resize(0, 0);
		return true;
	}

	std::string TemplateDetectStream::Name() const {
		return "TemplateDetectStream";
	}

	TemplateDetectStream::~TemplateDetectStream() {}

	void TemplateDetectStream::SetSensitivity(const std::string& sensitivities) {
		std::vector<float> parts;
		SplitStringToFloats(sensitivities, ",", &parts);
		if (parts.size() != field_x58.size()) {
			if (parts.size() == 1) {
				parts.resize(field_x58.size(), parts[0]);
			} else
				throw snowboy_exception{"Number of sensitivities does not match number of models ("
										+ std::to_string(parts.size()) + " v.s. " + std::to_string(m_models.size()) + ")"};
		}
		for (size_t i = 0; i < field_x58.size(); i++) {
			m_models[i].m_sensitivity = parts[i];
			for (auto& e : field_x58[i]) {
				e.SetEarlyStopThreshold(parts[i]);
			}
		}
	}

	std::string TemplateDetectStream::GetSensitivity() const {
		std::string res;
		for (size_t i = 0; i < m_models.size(); i++) {
			if (!res.empty()) res += ", ";
			res += std::to_string(m_models[i].m_sensitivity);
		}
		return res;
	}

	void TemplateDetectStream::InitDtw() {
		field_x58.resize(m_models.size());
		for (size_t i = 0; i < m_models.size(); i++) {
			auto ntemplates = m_models[i].NumTemplates();
			field_x58[i].resize(field_x58[i].size() + ntemplates);
			for (size_t t = 0; t < ntemplates; t++) {
				auto& e = field_x58[i][t];
				e.SetOptions(m_options.dtw_options);
				auto tmpl = m_models[i].GetTemplate(t);
				e.SetReference(tmpl);
				e.SetEarlyStopThreshold(m_models[i].m_sensitivity);
				field_x70 = std::max<size_t>(e.GetWindowSize(), field_x70);
			}
		}
	}

	size_t TemplateDetectStream::NumHotwords(size_t model_id) const {
		if (model_id >= m_models.size()) {
			throw snowboy_exception{"model id runs out of range, expecting a value between [0, "
									+ std::to_string(m_models.size()) + "] got " + std::to_string(model_id) + " instead."};
			return 0;
		}
		return 1;
	}

	void TemplateDetectStream::UpdateModel() const {
		std::vector<std::string> models;
		SplitStringToVector(m_options.model_str, ",", &models);
		for (size_t i = 0; i < models.size() && i < m_models.size(); i++) {
			m_models[i].WriteHotwordModel(true, models[i]);
		}
	}

} // namespace snowboy
