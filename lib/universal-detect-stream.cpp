#include <frame-info.h>
#include <limits>
#include <math.h>
#include <nnet-lib.h>
#include <snowboy-debug.h>
#include <snowboy-io.h>
#include <snowboy-options.h>
#include <universal-detect-stream.h>

namespace snowboy {
	void UniversalDetectStreamOptions::Register(const std::string& prefix, OptionsItf* opts) {
		opts->Register(prefix, "slide-step", "Step size for sliding window in frames.", &slide_step);
		opts->Register(prefix, "sensitivity-str", "String that contains the sensitivity value for each hotword, separated by comma.", &sensitivity_str);
		opts->Register(prefix, "high-sensitivity-str", "String that contains the higher sensitivity value for each hotword, separated by comma.", &high_sensitivity_str);
		opts->Register(prefix, "model-str", "String that contains hotword models, separated by comma. Note that each universal model may contain more than one hotword.", &model_str);
		opts->Register(prefix, "smooth-window-str", "String that contains smoothing window size in frames for each model, separated by comma.", &smooth_window_str);
		opts->Register(prefix, "slide-window-str", "String that contains sliding window size in frames for each model, separated by comma.", &slide_window_str);
		opts->Register(prefix, "min-detection-interval", "Minimal number of frames between two consecutive detections.", &min_detection_interval);
		opts->Register(prefix, "debug-mode", "If true, turns off things like order enforcing, and will print out more info.", &debug_mode);
		opts->Register(prefix, "min-num-frames-per-phone", "Minimal number of frames on each phone.", &min_num_frames_per_phone);
		opts->Register(prefix, "num-repeats", "For search method 4 only, number of repeats when search the hotword.", &num_repeats);
	}

	UniversalDetectStream::UniversalDetectStream(const UniversalDetectStreamOptions& options) {
		m_options = options;
		if (m_options.model_str == "") {
			SNOWBOY_ERROR() << "please specify models through --model-str.";
			return;
		}
		if (m_options.slide_step < 1) {
			SNOWBOY_ERROR() << "slide step size should be positive.";
			return;
		}
		field_x58 = m_options.min_detection_interval;
		field_x5c = m_options.min_detection_interval;
		// Note: in the original code there are resize(0) calls for each vector,
		// but this is not needed since they are constructed empty anyway.
		ReadHotwordModel(m_options.model_str);
		if (!m_options.smooth_window_str.empty()) SetSmoothWindowSize(m_options.smooth_window_str);
		if (!m_options.slide_window_str.empty()) SetSlideWindowSize(m_options.slide_window_str);
		if (!m_options.sensitivity_str.empty()) SetSensitivity(m_options.sensitivity_str);
		if (m_options.high_sensitivity_str.empty()) {
			if (!m_options.sensitivity_str.empty()) SetHighSensitivity(m_options.sensitivity_str);
		} else
			SetHighSensitivity(m_options.high_sensitivity_str);
		for (auto& e : m_model_info)
			e.CheckLicense();
		field_x60 = false;
		field_x64 = 0;
		field_x68 = false;
		field_x6c = 0;
	}

	int UniversalDetectStream::Read(Matrix* mat, std::vector<FrameInfo>* info) {
		mat->Resize(0, 0);
		if (info) info->clear();
		Matrix read_mat;
		std::vector<FrameInfo> read_info;
		auto read_res = m_connectedStream->Read(&read_mat, &read_info);
		if ((read_res & 0xc2) != 0) return read_res;
		for (size_t file = 0; file < m_model_info.size(); file++) {
			Matrix nnet_out_mat;
			std::vector<FrameInfo> nnet_out_info;
			if ((read_res & 0x18) == 0)
				m_model_info[file].field_x70.Compute(read_mat, read_info, &nnet_out_mat, &nnet_out_info);
			else
				m_model_info[file].field_x70.FlushOutput(read_mat, read_info, &nnet_out_mat, &nnet_out_info);
			m_model_info[file].SmoothPosterior(&nnet_out_mat);
			for (size_t r = 0; r < nnet_out_mat.m_rows; r += m_options.slide_step) {
				auto max = 0;
				if (r + m_options.slide_step > nnet_out_mat.m_rows)
					max = nnet_out_mat.m_rows;
				else
					max = r + m_options.slide_step;
				PushSlideWindow(file, nnet_out_mat.RowRange(r, max - r));
				const auto max_frame_id = nnet_out_info[max - 1].frame_id;
				float fVar8 = 0.0f;
				int local_130 = -1;
				for (size_t i = 0; i < m_model_info[file].keywords.size(); i++) {
					auto posterior = GetHotwordPosterior(file, i, max_frame_id);
					if (!field_x68 || max_frame_id - field_x6c < 0x33) {
						if (field_x60) {
							if (3000 < max_frame_id - field_x64) {
								field_x60 = false;
							}
							if (1.0f - m_model_info[file].keywords[i].field_xb8 <= posterior && m_options.min_detection_interval < max_frame_id - field_x58)
							{
								if (fVar8 < posterior) {
									local_130 = i;
									fVar8 = posterior;
								}
								field_x64 = max_frame_id;
							}
						} else {
							if (posterior < 1.0f - m_model_info[file].keywords[i].field_xa0 || max_frame_id - field_x58 <= m_options.min_detection_interval) {
								if (!field_x68
									&& 1.0f - m_model_info[file].keywords[i].field_xb8 <= posterior
									&& posterior < 1.0f - m_model_info[file].keywords[i].field_xa0
									&& max_frame_id - field_x58 <= m_options.min_detection_interval) {
									field_x68 = true;
									field_x6c = max_frame_id;
								}
							} else {
								if (fVar8 < posterior) {
									local_130 = i;
									fVar8 = posterior;
								}
								if (!field_x68 && m_model_info[file].keywords[i].field_xa0 < m_model_info[file].keywords[i].field_xb8) {
									field_x68 = true;
									field_x6c = max_frame_id;
								}
							}
						}
					} else {
						field_x68 = false;
						field_x60 = true;
						field_x64 = max_frame_id;
						if (1.0f - m_model_info[file].keywords[i].field_xb8 <= posterior && m_options.min_detection_interval < max_frame_id - field_x58)
						{
							if (fVar8 < posterior) {
								local_130 = i;
								fVar8 = posterior;
							}
							field_x64 = max_frame_id;
						}
					}
				}
				if (local_130 != -1) {
					m_model_info[file].CheckLicense();
					field_x58 = max_frame_id;
					field_x5c = max_frame_id;
					ResetDetection();
					mat->Resize(1, 1);
					mat->m_data[0] = m_model_info[file].keywords[local_130].field_xd0;
					if (info != nullptr) {
						auto i = nnet_out_info[r];
						info->push_back(i);
						return read_res;
					}
				}
			}
		}
		if ((read_res & 0x18) != 0) {
			this->Reset();
		}
		return read_res;
	}

	bool UniversalDetectStream::Reset() {
		for (auto& e : m_model_info)
			e.field_x70.ResetComputation();
		ResetDetection();
		return true;
	}

	std::string UniversalDetectStream::Name() const {
		return "UniversalDetectStream";
	}

	UniversalDetectStream::~UniversalDetectStream() {}

	void UniversalDetectStream::ModelInfo::CheckLicense() const {
		if (field_x1a8 > 0.0f) {
			time_t t;
			time(&t);
			auto diff = difftime(t, field_x190);
			auto expires = field_x1a8;
			if (expires < (diff / 86400.0f)) {
				SNOWBOY_ERROR() << "Your license for Snowboy has been expired. Please contact KITT.AI at snowboy@kitt.ai";
				return;
			}
		}
	}

	float UniversalDetectStream::GetHotwordPosterior(int param_1, int param_2, int param_3) {
		switch (m_model_info[param_1].keywords[param_2].field_xe8) {
		case 1: return m_model_info[param_1].HotwordNaiveSearch(param_2);
		case 2: return HotwordDtwSearch(param_1, param_2);
		case 3: return HotwordViterbiSearch(param_1, param_2);
		case 4: return HotwordPiecewiseSearch(param_1, param_2);
		case 5: return HotwordViterbiSearchReduplication(param_1, param_2, param_3);
		case 6: return HotwordViterbiSearchSoftFloor(param_1, param_2);
		case 7: return HotwordViterbiSearchTraceback(param_1, param_2);
		case 8: return HotwordViterbiSearchTracebackLog(param_1, param_2);
		default: {
			SNOWBOY_ERROR() << "search method has not been implemented.";
			return 0.0f;
		}
		}
	}

	std::string UniversalDetectStream::GetSensitivity() const {
		std::stringstream res;
		for (size_t i = 0; i < m_model_info.size(); i++) {
			for (size_t x = 0; x < m_model_info[i].keywords.size(); x++) {
				if (i != 0 || x != 0) {
					res << ", ";
				}
				res << m_model_info[i].keywords[x].field_xa0;
			}
		}
		return res.str();
	}

	float UniversalDetectStream::HotwordDtwSearch(int, int) const {
		SNOWBOY_ERROR() << "Not implemented";
		return 0.0f;
		// TODO:: This is unused in all models I have, but we should still implement it at some point
	}

	float UniversalDetectStream::ModelInfo::HotwordNaiveSearch(int param_2) const {
		float sum = 0.0f;
		for (size_t i = 0; i < keywords[param_2].field_x88.size(); i++) {
			auto& x = field_x250[keywords[param_2].field_x88[i]];
			if (keywords[param_2].field_x160[i] > x.front()) return 0.0f;
			sum += logf(std::max(x.front(), std::numeric_limits<float>::min()));
		}
		return expf(sum / static_cast<float>(keywords[param_2].field_x88.size()));
	}

	float UniversalDetectStream::HotwordPiecewiseSearch(int, int) const {
		SNOWBOY_ERROR() << "Not implemented";
		return 0.0f;
		// TODO:: This is unused in all models I have, but we should still implement it at some point
	}

	float UniversalDetectStream::HotwordViterbiSearch(int param_1, int param_2) const {
		return m_model_info[param_1].HotwordNaiveSearch(param_2);
		// TODO: Implement Viterbi search
		std::vector<float> x;
		x.resize(m_model_info[param_1].keywords[param_2].field_x88.size(), -std::numeric_limits<float>::max());
		x[0] = 0.0f;
		std::vector<float> x2;
		x2.resize(m_model_info[param_1].keywords[param_2].field_x88.size(), 0);
		auto& f250 = m_model_info[param_1].field_x250[0];
		int i = f250.size() - m_model_info[param_1].keywords[param_2].field_x148.back();
		do {
			if (f250.size() <= i) {
				auto fVar2 = m_model_info[param_1].keywords[param_2].field_x160.back();
				if (fVar2 <= x2.back()) {
					if (m_model_info[param_1].keywords[param_2].field_x178 && !x.empty()) {
						for (auto& e : x) {
							fVar2 = std::max(e, fVar2);
						}
						return fVar2 / static_cast<float>(m_model_info[param_1].keywords[param_2].field_x148.back());
					}
				}
			}
			if (!x.empty()) {
			}
			i++;
		} while (true);
		// TODO: Implement Viterbi search
	}

	float UniversalDetectStream::HotwordViterbiSearch(int, int, int, const PieceInfo&) const {
		SNOWBOY_ERROR() << "Not implemented";
		return 0.0f;
		// TODO:: This is unused in all models I have, but we should still implement it at some point
	}

	float UniversalDetectStream::HotwordViterbiSearchReduplication(int, int, int) {
		SNOWBOY_ERROR() << "Not implemented";
		return 0.0f;
		// TODO:: This is unused in all models I have, but we should still implement it at some point
	}

	float UniversalDetectStream::HotwordViterbiSearchSoftFloor(int, int) const {
		SNOWBOY_ERROR() << "Not implemented";
		return 0.0f;
		// TODO:: This is unused in all models I have, but we should still implement it at some point
	}

	float UniversalDetectStream::HotwordViterbiSearchTraceback(int, int) const {
		SNOWBOY_ERROR() << "Not implemented";
		return 0.0f;
		// TODO:: This is unused in all models I have, but we should still implement it at some point
	}

	float UniversalDetectStream::HotwordViterbiSearchTracebackLog(int param_1, int param_2) const {
		// TODO: Implement this
		return m_model_info[param_1].HotwordNaiveSearch(param_2);
	}

	int UniversalDetectStream::ModelInfo::NumHotwords() const {
		return keywords.size();
	}

	int UniversalDetectStream::NumHotwords(int model_id) const {
		if (model_id < m_model_info.size() && model_id >= 0) {
			return m_model_info[model_id].NumHotwords();
		} else {
			SNOWBOY_ERROR() << "model_id runs out of range, expecting a value between [0," << m_model_info.size() << "], got " << model_id << " instead.";
			return 0;
		}
	}

	void UniversalDetectStream::PushSlideWindow(int param_1, const MatrixBase& param_2) {
		// TODO: Optimize this by calculating offsets and doing a memcpy
		for (size_t r = 0; r < param_2.m_rows; r++) {
			for (size_t c = 0; c < param_2.m_cols; c++) {
				m_model_info[param_1].field_x250[c].push_back(param_2.m_data[r * param_2.m_stride + c]);
				if (m_model_info[param_1].field_x250[c].size() > m_model_info.size()) {
					m_model_info[param_1].field_x250[c].pop_front();
				}
			}
		}
	}

	void UniversalDetectStream::KeyWordInfo::ReadKeyword(bool binary, std::istream* is, int slide_window) {
		ExpectToken(binary, "<Kw>", is);
		ReadIntegerVector(binary, &field_x88, is);
		ExpectToken(binary, "<Sensitivity>", is);
		ReadBasicType<float>(binary, &field_xa0, is);
		field_xb8 = 0.0f;
		field_x160.resize(field_x88.size());
		// TODO: I think there is a bug here.
		// If SearchMax is present, but SearchMethod is not, the code would branch into the first
		// if and throw on the ExpectToken. It might be possible that SearchMethod is *required*
		// if SearchMax is present, but I dont know for sure.
		if (PeekToken(binary, is) == 'S') {
			ExpectToken(binary, "<SearchMethod>", is);
			ReadBasicType<int>(binary, &field_xe8, is);
			ExpectToken(binary, "<SearchNeighbour>", is);
			ReadBasicType<int>(binary, &field_x100, is);
			ExpectToken(binary, "<SearchMask>", is);
			ReadIntegerVector<int>(binary, &field_x148, is);
			ExpectToken(binary, "<SearchFloor>", is);
			Vector tvec;
			tvec.Read(binary, is);
			field_x160.resize(tvec.m_size);
			for (size_t i = 0; i < tvec.m_size; i++)
				field_x160[i] = tvec.m_data[i];
		} else {
			field_x148.resize(field_x88.size());
			for (size_t i = 0; i < field_x148.size(); i++) {
				field_x148[i] = (static_cast<float>(i) / static_cast<float>(field_x148.size())) * static_cast<float>(slide_window);
			}
		}
		if (PeekToken(binary, is) == 'S') {
			ExpectToken(binary, "<SearchMax>", is);
			ReadBasicType<bool>(binary, &field_x178, is);
		}
		if (PeekToken(binary, is) == 'N') {
			ExpectToken(binary, "<NumPieces>", is);
			ReadBasicType<int>(binary, &field_x1d8, is);
		}
		if (PeekToken(binary, is) == 'D') {
			ExpectToken(binary, "<DurationPass>", is);
			ReadBasicType<int>(binary, &field_x118, is);
			ExpectToken(binary, "<FloorPass>", is);
			ReadBasicType<int>(binary, &field_x130, is);
		}
	}

	void UniversalDetectStream::ModelInfo::ReadHotwordModel(bool binary, std::istream* is, int num_repeats, int* hotword_id) {
		ExpectToken(binary, "<UniversalModel>", is);
		if (PeekToken(binary, is) == 'L') {
			ExpectToken(binary, "<LicenseStart>", is);
			ReadBasicType<long>(binary, &field_x190, is);
			ExpectToken(binary, "<LicenseDays>", is);
			ReadBasicType<float>(binary, &field_x1a8, is);
		} else {
			field_x190 = 0;
			field_x1a8 = 0.0f;
		}
		ExpectToken(binary, "<KwInfo>", is);
		ExpectToken(binary, "<SmoothWindow>", is);
		ReadBasicType<int>(binary, &field_x208, is);
		ExpectToken(binary, "<SlideWindow>", is);
		ReadBasicType<int>(binary, &field_x220, is);
		ExpectToken(binary, "<NumKws>", is);
		int num_kws;
		ReadBasicType<int>(binary, &num_kws, is);
		keywords.resize(num_kws);
		for (auto& e : keywords) {
			e.field_xe8 = 1;
			e.field_x1c0 = num_repeats;
			e.field_x1d8 = 1;
		}
		for (size_t kw = 0; kw < num_kws; kw++) {
			keywords[kw].ReadKeyword(binary, is, field_x220);
			keywords[kw].field_xd0 = (*hotword_id)++;
		}
		ExpectToken(binary, "</KwInfo>", is);
		field_x70.Read(binary, is);
		field_x238.resize(field_x238.size() + field_x70.OutputDim());
		field_x250.resize(field_x250.size() + field_x70.OutputDim());
		field_x268.resize(field_x268.size() + field_x70.OutputDim());
		if (keywords[0].field_xe8 == 4) {
			SNOWBOY_ERROR() << "Not implemented!";
			// TODO
		}
	}

	void UniversalDetectStream::ReadHotwordModel(const std::string& filename) {
		std::vector<std::string> files;
		SplitStringToVector(filename, global_snowboy_string_delimiter, &files);
		if (files.empty()) {
			SNOWBOY_ERROR() << "no model can be extracted from --model-str: " << filename;
			return;
		}
		auto s = files.size();
		m_model_info.resize(s);

		int hotword_id = 1;
		for (size_t f = 0; f < files.size(); f++) {
			Input in{files[f]};
			auto binary = in.is_binary();
			auto is = in.Stream();
			m_model_info[f].ReadHotwordModel(binary, is, m_options.num_repeats, &hotword_id);
		}
	}

	void UniversalDetectStream::ModelInfo::ResetDetection() {
		for (size_t x = 0; x < field_x238.size(); x++) {
			field_x238[x].clear();
		}
		for (size_t x = 0; x < field_x250.size(); x++) {
			field_x250[x].clear();
		}
		for (size_t x = 0; x < field_x268.size(); x++) {
			field_x268[x] = 0.0f;
		}
		for (size_t x = 0; x < keywords.size(); x++) {
			keywords[x].field_x280 = false;
		}
		for (size_t x = 0; x < field_x2b0.size(); x++) {
			field_x2b0[x] = 0.0f;
		}
		for (size_t x = 0; x < keywords.size(); x++) {
			keywords[x].field_x298 = -1000;
		}
	}

	void UniversalDetectStream::ResetDetection() {
		for (auto& e : m_model_info) {
			e.ResetDetection();
		}
	}

	void UniversalDetectStream::SetHighSensitivity(const std::string& param_1) {
		std::vector<float> parts;
		SplitStringToFloats(param_1, global_snowboy_string_delimiter, &parts);
		if (parts.size() == 1) {
			for (auto& e : m_model_info) {
				for (auto& e2 : e.keywords) {
					e2.field_xb8 = parts[0];
				}
			}
		} else if (parts.size() == m_model_info.size()) {
			for (size_t i = 0; i < m_model_info.size(); i++) {
				for (auto& e : m_model_info[i].keywords) {
					e.field_xb8 = parts[i];
				}
			}
		} else {
			SNOWBOY_ERROR() << "Number of sensitivities does not match number of hotwords ("
							<< parts.size() << " v.s. " << m_model_info.size()
							<< "). Note that each universal model may have multiple hotwords.";
			return;
		}
	}

	void UniversalDetectStream::SetSensitivity(const std::string& param_1) {
		std::vector<float> parts;
		SplitStringToFloats(param_1, global_snowboy_string_delimiter, &parts);
		if (parts.size() == 1) {
			for (auto& e : m_model_info) {
				for (auto& e2 : e.keywords) {
					e2.field_xa0 = parts[0];
				}
			}
		} else if (parts.size() == m_model_info.size()) {
			for (size_t i = 0; i < m_model_info.size(); i++) {
				for (auto& e : m_model_info[i].keywords) {
					e.field_xa0 = parts[i];
				}
			}
		} else {
			SNOWBOY_ERROR() << "Number of sensitivities does not match number of hotwords ("
							<< parts.size() << " v.s. " << m_model_info.size()
							<< "). Note that each universal model may have multiple hotwords.";
			return;
		}
	}

	void UniversalDetectStream::SetSlideWindowSize(const std::string& param_1) {
		std::vector<int> parts;
		SplitStringToIntegers<int>(param_1, global_snowboy_string_delimiter, &parts);
		for (size_t i = 0; i < std::min(m_model_info.size(), parts.size()); i++) {
			m_model_info[i].field_x220 = parts[i];
		}
	}

	void UniversalDetectStream::SetSmoothWindowSize(const std::string& param_1) {
		std::vector<int> parts;
		SplitStringToIntegers<int>(param_1, global_snowboy_string_delimiter, &parts);
		for (size_t i = 0; i < std::min(m_model_info.size(), parts.size()); i++) {
			m_model_info[i].field_x208 = parts[i];
		}
	}

	void UniversalDetectStream::ModelInfo::SmoothPosterior(Matrix* param_2) {
		for (size_t r = 0; r < param_2->m_rows; r++) {
			for (size_t c = 0; c < param_2->m_cols; c++) {
				auto val = param_2->m_data[r * param_2->m_stride + c];
				field_x268[c] += val;
				field_x238[c].push_back(val);
				if (field_x238[c].size() > field_x208) {
					field_x238[c].pop_front();
				}
				param_2->m_data[r * param_2->m_stride + c] = field_x268[c] / field_x208;
			}
		}
	}

	void UniversalDetectStream::ModelInfo::UpdateLicense(long param_2, float param_3) {
		field_x190 = param_2;
		field_x1a8 = param_3;
	}

	void UniversalDetectStream::UpdateLicense(int param_1, long param_2, float param_3) {
		m_model_info[param_1].UpdateLicense(param_2, param_3);
	}

	void UniversalDetectStream::UpdateModel() const {
		WriteHotwordModel(true, m_options.model_str);
	}

	void UniversalDetectStream::KeyWordInfo::WriteKeyword(bool binary, std::ostream* os) const {
		WriteToken(binary, "<Kw>", os);
		WriteIntegerVector<int>(binary, field_x88, os);
		WriteToken(binary, "<Sensitivity>", os);
		WriteBasicType<float>(binary, field_xa0, os);
		WriteToken(binary, "<SearchMethod>", os);
		WriteBasicType<int>(binary, field_xe8, os);
		WriteToken(binary, "<SearchNeighbour>", os);
		WriteBasicType<int>(binary, field_x100, os);
		WriteToken(binary, "<SearchMask>", os);
		WriteIntegerVector<int>(binary, field_x148, os);
		WriteToken(binary, "<SearchFloor>", os);
		Vector tvec;
		tvec.Resize(field_x160.size());
		// TODO: This could be a memcpy, or even better implement writing for vector
		for (size_t i = 0; i < tvec.m_size; i++) {
			tvec.m_data[i] = field_x160[i];
		}
		tvec.Write(binary, os);
		WriteToken(binary, "<SearchMax>", os);
		WriteBasicType<bool>(binary, field_x178, os);
		WriteToken(binary, "<NumPieces>", os);
		WriteBasicType<int>(binary, field_x1d8, os);
		WriteToken(binary, "<DurationPass>", os);
		WriteBasicType<int>(binary, field_x118, os);
		WriteToken(binary, "<FloorPass>", os);
		WriteBasicType<int>(binary, field_x130, os);
	}

	void UniversalDetectStream::ModelInfo::WriteHotwordModel(bool binary, std::ostream* os) const {
		WriteToken(binary, "<UniversalModel>", os);
		WriteToken(binary, "<LicenseStart>", os);
		WriteBasicType<long>(binary, field_x190, os);
		WriteToken(binary, "<LicenseDays>", os);
		WriteBasicType<float>(binary, field_x1a8, os);
		WriteToken(binary, "<KwInfo>", os);
		WriteToken(binary, "<SmoothWindow>", os);
		WriteBasicType<int>(binary, field_x208, os);
		WriteToken(binary, "<SlideWindow>", os);
		WriteBasicType<int>(binary, field_x220, os);
		WriteToken(binary, "<NumKws>", os);
		WriteBasicType<int>(binary, keywords.size(), os);
		for (size_t kw = 0; kw < keywords.size(); kw++) {
			keywords[kw].WriteKeyword(binary, os);
		}
		WriteToken(binary, "</KwInfo>", os);
		field_x70.Write(binary, os);
	}

	void UniversalDetectStream::WriteHotwordModel(bool binary, const std::string& filename) const {
		std::vector<std::string> parts;
		SplitStringToVector(filename, global_snowboy_string_delimiter, &parts);
		for (size_t file = 0; file < parts.size(); file++) {
			Output out{parts[file], binary};
			auto os = out.Stream();
			m_model_info[file].WriteHotwordModel(binary, os);
		}
	}

} // namespace snowboy
