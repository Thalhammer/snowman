#include <cmath>
#include <frame-info.h>
#include <limits>
#include <raw-energy-vad-stream.h>
#include <snowboy-options.h>
#include <vector-wrapper.h>

namespace snowboy {
	void RawEnergyVadStreamOptions::Register(const std::string& prefix, OptionsItf* opts) {
		opts->Register(prefix, "init-bg-energy", "If true, initializes the background log energy from the first "
												 "--bg-buffer-sizeframes, otherwise sets it to 0.",
					   &init_bg_energy);
		opts->Register(prefix, "bg-energy-threshold", "Threshold for energy VAD.", &bg_energy_threshold);
		opts->Register(prefix, "bg-buffer-size", "Number of buffered frames for computing background log energy.", &bg_buffer_size);
		opts->Register(prefix, "raw-buffer-extra", "Number of extra frames to be buffered in the raw energy buffer. "
												   "This takes care ofthe frame delays when calling UpdateBackgroundEnergy().",
					   &raw_buffer_extra);
		opts->Register(prefix, "bg-energy-cap", "Cap of background energy, so that the energy VAD will not block the detection.", &bg_energy_cap);
	}

	RawEnergyVadStream::RawEnergyVadStream(const RawEnergyVadStreamOptions& options) {
		m_options = options;
		Reset();
	}

	int RawEnergyVadStream::Read(Matrix* mat, std::vector<FrameInfo>* info) {
		auto sig = m_connectedStream->Read(mat, info);
		if ((sig & 0xc2) != 0) {
			mat->Resize(0, 0);
			info->clear();
			return sig;
		}
		if (!field_x2c) {
			InitRawEnergyVad(mat, info);
		} else {
			for (size_t r = 0; r < mat->rows(); r++) {
				auto dot = SubVector{*mat, r}.DotVec(SubVector{*mat, r});
				dot = std::max(std::numeric_limits<float>::min(), dot);
				dot = logf(dot);
				auto energy = dot - m_bg_energy;
				if (energy > m_options.bg_energy_threshold) {
					info->at(r).flags |= 0x1;
				} else {
					info->at(r).flags &= ~0x1;
				}
				field_x38.push_back({info->at(r).frame_id, dot});
			}
			while (field_x38.size() > mat->rows() + m_options.raw_buffer_extra) {
				field_x38.pop_front();
			}
		}
		if ((sig & 0x18) != 0 && m_someMatrix.rows() != 0) {
			mat->Swap(&m_someMatrix);
			info->swap(field_xf0);
		}
		return sig;
	}

	bool RawEnergyVadStream::Reset() {
		m_bg_energy = 0;
		field_x34 = 0;
		field_x2c = m_options.init_bg_energy ^ 1;
		field_x38.clear();
		field_x88.clear();
		m_someMatrix.Resize(0, 0);
		field_xf0.clear();
		return true;
	}

	std::string RawEnergyVadStream::Name() const {
		return "RawEnergyVadStream";
	}

	RawEnergyVadStream::~RawEnergyVadStream() {}

	void RawEnergyVadStream::InitRawEnergyVad(Matrix* mat, std::vector<FrameInfo>* info) {
		if (mat->m_rows == 0) return;
		auto rows = m_someMatrix.m_rows;
		m_someMatrix.Resize(mat->m_rows + rows, mat->m_cols, MatrixResizeType::kCopyData);
		m_someMatrix.RowRange(rows, mat->m_cols).CopyFromMat(*mat, MatrixTransposeType::kNoTrans);
		field_xf0.reserve(field_xf0.size() + info->size());
		for (auto& e : *info)
			field_xf0.push_back(e);
		mat->Resize(0, 0);
		info->clear();
		if (m_someMatrix.m_rows >= m_options.bg_buffer_size) {
			field_x38.resize(m_someMatrix.rows());
			for (size_t r = 0; r < m_someMatrix.rows(); r++) {
				auto dot = SubVector{m_someMatrix, r}.DotVec(SubVector{m_someMatrix, r});
				dot = std::max(std::numeric_limits<float>::min(), dot);
				dot = logf(dot);
				auto& e = field_x38[r];
				e.first = field_xf0[r].frame_id;
				e.second = dot;
			}
			m_bg_energy = 0.0;
			for (size_t i = m_options.bg_buffer_size / 2; i < field_x38.size(); i++) {
				m_bg_energy += field_x38[i].second;
			}
			auto s = static_cast<ssize_t>(field_x38.size()) - m_options.bg_buffer_size / 2;
			if (s < 0) {
				s *= 2;
			}
			m_bg_energy /= static_cast<float>(s);
			m_bg_energy = std::min(m_options.bg_energy_cap, m_bg_energy);
			for (size_t i = m_options.bg_buffer_size / 2; i < field_x38.size(); i++) {
				if (field_x38[i].second - m_bg_energy > m_options.bg_energy_threshold)
					field_xf0[i].flags |= 1;
				else
					field_xf0[i].flags &= ~1;
			}
			mat->Swap(&m_someMatrix);
			field_xf0.swap(*info);
			field_x2c = true;
		}
	}

	void RawEnergyVadStream::UpdateBackgroundEnergy(const std::vector<FrameInfo>& info) {
		if (info.size() != 0) {
			auto ppVar7 = field_x38.begin();
			while (field_x38.size() != 0) {
				if (info[0].frame_id <= ppVar7->first) {
					for (size_t uVar20 = 0; uVar20 < info.size(); uVar20++) {
						if (info[uVar20].frame_id == ppVar7->first) {
							if ((info[uVar20].flags & 1) == 0) {
								field_x88.push_back(ppVar7->second);
							}
							ppVar7 = field_x38.erase(ppVar7);
						}
					}
					auto pfVar6 = field_x88.begin();
					while (m_options.bg_buffer_size < field_x88.size()) {
						this->field_x34 = this->field_x34 - *pfVar6;
						pfVar6 = field_x88.erase(pfVar6);
					}
					if (m_options.bg_buffer_size != field_x88.size()) {
						return;
					}
					m_bg_energy = std::min(field_x34 / (float)m_options.bg_buffer_size, m_options.bg_energy_cap);
					return;
				}
				ppVar7 = field_x38.erase(ppVar7);
			}
		}
	}

} // namespace snowboy
