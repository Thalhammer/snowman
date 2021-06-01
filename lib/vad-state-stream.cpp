#include <frame-info.h>
#include <snowboy-io.h>
#include <snowboy-options.h>
#include <vad-lib.h>
#include <vad-state-stream.h>

namespace snowboy {

	void VadStateStreamOptions::Register(const std::string& prefix, OptionsItf* opts) {
		opts->Register(prefix, "min-non-voice-frames", "Minimal number of non-voice frames to be accumulated before jumping into a non-voice state.", &min_non_voice_frames);
		opts->Register(prefix, "min-voice-frames", "Minimal number of voice frames to be accumulated before jumping into a voice state.", &min_voice_frames);
		opts->Register(prefix, "remove-non-voice", "If true, remove non-voice frames.", &remove_non_voice);
		opts->Register(prefix, "extra-frame-adjust", "Adjustment to the number of extra frames to the left of a voiced segment.", &extra_frame_adjust);
	}

	int VadStateStream::ProcessCachedSignal(Matrix* mat, std::vector<FrameInfo>* info) {
		Matrix local_68;
		std::vector<FrameInfo> local_48;
		int res = 1;
		if (field_x50.empty()) {
			mat->Resize(0, 0);
			info->clear();
		} else {
			res = ProcessDataAndInfo(local_68, local_48, mat, info);
		}
		if (field_x50.empty()) {
			if ((field_xa0 & 8) != 0) {
				if (field_x30 == 0)
					field_xa0 &= ~8;
				else {
					field_x30 = 0;
					field_xa4 = 2;
				}
				m_someOtherMatrix.Resize(0, 0);
				field_x80.clear();
			}
			res |= field_xa0;
			field_xa0 = 1;
		}
		PrintVlog(static_cast<SnowboySignal>(res), *info);
		return res;
	}

	int VadStateStream::ProcessDataAndInfo(const MatrixBase& param_1, const std::vector<FrameInfo>& param_2, Matrix* param_3, std::vector<FrameInfo>* param_4) {
		// TODO: The jumps in this function most likely where no jumps in the original,
		// TODO: but I could not figure out a nice loop condition for them, so they stay for now
		// TODO: Ideas are obviously welcome ;)
		auto rows = m_someMatrix.m_rows + param_1.m_rows;
		if (rows == 0) {
			param_3->Resize(0, 0);
			param_4->clear();
			return 1;
		}
		auto cols = param_1.m_cols < 1 ? m_someMatrix.m_cols : param_1.m_cols;
		Matrix local_b8;
		local_b8.Resize(rows, cols);
		if (m_someMatrix.m_rows > 0) {
			local_b8.RowRange(0, m_someMatrix.m_rows).CopyFromMat(m_someMatrix, MatrixTransposeType::kNoTrans);
		}
		if (param_1.m_rows > 0) {
			local_b8.RowRange(m_someMatrix.m_rows, param_1.m_rows).CopyFromMat(param_1, MatrixTransposeType::kNoTrans);
		}
		m_someMatrix.Resize(0, 0);
		std::vector<FrameInfo> tinfo;
		tinfo.reserve(param_2.size() + field_x50.size());
		for (auto& e : field_x50)
			tinfo.push_back(e);
		for (auto& e : param_2)
			tinfo.push_back(e);
		field_x50.clear();
		int local_c8 = 1;
		size_t lVar7 = 0;
		if (!tinfo.empty()) {
			size_t uVar4 = 0;
			size_t uVar10 = 0;
			if (field_xa4 == 2) goto LAB_0016b789;
			while (field_xa4 != 1 || (tinfo[uVar4].flags & 1) != 0) {
				while (true) {
					if (uVar4 == tinfo.size() - 1) {
						uVar10 = tinfo.size();
						this->field_xa4 = 2 - (tinfo.back().flags & 1U);
					}
					uVar4 += 1;
					if (uVar4 == tinfo.size()) {
						lVar7 = uVar10;
						goto LAB_0016b799;
					}
					lVar7 = uVar4;
					if (field_xa4 != 2) break;
				LAB_0016b789:
					if ((tinfo[uVar4].flags & 1) != 0) {
						if (lVar7 != 0) goto LAB_0016b799;
						field_x30 = true;
						field_xa4 = 1;
						local_c8 = 5;
					}
				}
			}
			field_x30 = false;
			field_xa4 = 2;
			local_c8 |= 8;
		}
	LAB_0016b799:
		if (this->m_options.remove_non_voice == false || (local_c8 & 0xc) != 0 || (tinfo[lVar7 - 1].flags & 1) != 0) {
		LAB_0016b7aa:
			if (0 < lVar7) {
			LAB_0016b7b3:
				if ((this->m_options).remove_non_voice == false) {
					param_3->Resize(lVar7, local_b8.m_cols);
					param_3->CopyFromMat(local_b8.RowRange(0, lVar7), MatrixTransposeType::kNoTrans);
					param_4->resize(lVar7);
					for (size_t i = 0; i < param_4->size(); i++) {
						(*param_4)[i] = tinfo[i];
					}
					// Returns with some cleanup
					goto LAB_0016b940;
				}
				// Returns with cleanup and output clear
				if ((tinfo[lVar7 - 1].flags & 1) == 0) {
					param_3->Resize(0, 0);
					param_4->clear();
					goto LAB_0016b940;
				}
				if ((local_c8 & 4) == 0) {
					/* try { // try from 0016badb to 0016bb8f has its CatchHandler @ 0016c1d5 */
					param_3->Resize(lVar7, local_b8.m_cols);
					param_3->CopyFromMat(local_b8.RowRange(0, lVar7), MatrixTransposeType::kNoTrans);
					param_4->resize(lVar7);
					for (size_t i = 0; i < param_4->size(); i++) {
						(*param_4)[i] = tinfo[i];
					}
					// Returns with some cleanup
					goto LAB_0016b940;
				}
				param_3->Resize(m_someOtherMatrix.m_rows + lVar7, local_b8.m_cols);
				param_3->RowRange(0, m_someOtherMatrix.m_rows).CopyFromMat(m_someOtherMatrix, MatrixTransposeType::kNoTrans);
				param_3->RowRange(m_someOtherMatrix.m_rows, lVar7).CopyFromMat(local_b8.RowRange(0, lVar7), MatrixTransposeType::kNoTrans);
				m_someOtherMatrix.Resize(0, 0);
				param_4->resize(lVar7 + field_x80.size());
				for (size_t lVar5 = 0; lVar5 < field_x80.size(); lVar5++) {
					(*param_4)[lVar5] = field_x80[lVar5];
				}
				for (size_t lVar5 = 0; lVar5 < lVar7; lVar5++) {
					(*param_4)[field_x80.size() + lVar5] = tinfo[lVar5];
				}
				field_x80.clear();
				// Returns with some cleanup
				goto LAB_0016b940;
			}
		} else {
			if (field_x28 <= lVar7) {
				m_someOtherMatrix.Resize(field_x28, local_b8.m_cols, MatrixResizeType::kUndefined);
				m_someOtherMatrix.CopyFromMat(local_b8.RowRange(lVar7 - field_x28, field_x28), MatrixTransposeType::kNoTrans);
				field_x80.resize(field_x28);
				for (size_t lVar5 = 0; lVar5 != field_x28; lVar5++) {
					field_x80[lVar5] = tinfo[lVar5 + lVar7 - field_x28];
				}
				// TODO: This might be a continue of the loop
				goto LAB_0016b7aa;
			}
			if (field_x28 <= m_someOtherMatrix.rows() + lVar7) {
				const auto iVar2 = field_x28 - lVar7;
				Matrix local_98;
				local_98.Resize(field_x28, local_b8.m_cols);
				local_98.RowRange(0, iVar2).CopyFromMat(m_someOtherMatrix.RowRange(m_someOtherMatrix.m_rows - iVar2, iVar2), MatrixTransposeType::kNoTrans);
				local_98.RowRange(iVar2, lVar7).CopyFromMat(local_b8.RowRange(0, lVar7), MatrixTransposeType::kNoTrans);
				m_someOtherMatrix.Swap(&local_98);
				std::vector<FrameInfo> pFVar6;
				pFVar6.resize(field_x28);
				auto pfVar15 = field_x80.size() - iVar2;
				for (size_t pfVar5 = 0; iVar2 != pfVar5; pfVar5++) {
					pFVar6[pfVar5] = field_x80[pfVar15 + pfVar5];
				}
				for (size_t i = 0; i < lVar7; i++) {
					pFVar6[iVar2 + i] = tinfo[i];
				}
				field_x80 = std::move(pFVar6);
				// TODO: This might be a continue of the loop
				goto LAB_0016b7aa;
			}
			if (0 < lVar7) {
				m_someOtherMatrix.Resize(lVar7, local_b8.m_cols, MatrixResizeType::kCopyData);
				m_someOtherMatrix.RowRange(m_someOtherMatrix.m_rows - lVar7, lVar7).CopyFromMat(local_b8.RowRange(0, lVar7), MatrixTransposeType::kNoTrans);
				auto old_size = field_x80.size();
				field_x80.resize(lVar7 + field_x80.size());
				for (size_t lVar5 = 0; lVar5 < lVar7; lVar5++) {
					field_x80[old_size + lVar5] = tinfo[lVar5];
				}
				// TODO: This might be a continue of the loop
				goto LAB_0016b7b3;
			}
		}
		param_3->Resize(0, 0);
		param_4->clear();
	LAB_0016b940:
		if (local_b8.m_rows - lVar7 > 0) {
			m_someMatrix.Resize(local_b8.m_rows - lVar7, local_b8.m_cols);
			m_someMatrix.CopyFromMat(local_b8.RowRange(lVar7, local_b8.m_rows - lVar7), MatrixTransposeType::kNoTrans);
			field_x50.resize(tinfo.size() - lVar7);
			if (tinfo.size() - lVar7 > 7) {
				for (size_t i = 0; i < tinfo.size() - lVar7; i++) {
					field_x50[i] = tinfo[i + lVar7];
				}
			}
		}
		return local_c8;
	}

	VadStateStream::VadStateStream(const VadStateStreamOptions& options)
		: m_options{options}, field_x28{std::max(0u, m_options.extra_frame_adjust + m_options.min_voice_frames)},
		  field_x2c{UINT32_MAX}, field_x30{0}, m_vadstate{new VadState({m_options.min_non_voice_frames, m_options.min_voice_frames})},
		  field_xa0{1}, field_xa4{2} {}

	int VadStateStream::Read(Matrix* mat, std::vector<FrameInfo>* info) {
		if (field_xa0 != 1) {
			return ProcessCachedSignal(mat, info);
		}
		Matrix local_b8;
		std::vector<FrameInfo> local_98;
		auto uVar6 = m_connectedStream->Read(&local_b8, &local_98);
		if ((uVar6 & 4) != 0) uVar6 = uVar6 & 0xfffffffb;
		if ((uVar6 & 0xc2) != 0) {
			mat->Resize(0, 0);
			info->clear();
			return uVar6;
		}
		if (!local_98.empty()) {
			std::vector<VoiceType> local_78;
			local_78.resize(local_98.size());
			std::vector<VoiceStateType> local_58;
			for (size_t i = 0; i < local_98.size(); i++) {
				local_78[i] = (local_98[i].flags & 1) ? VT_1 : VT_2;
			}
			m_vadstate->GetVoiceStates(local_78, &local_58);
			for (size_t i = 0; i < local_98.size(); i++) {
				if (local_58[i] == VST_1)
					local_98[i].flags |= 1;
				else
					local_98[i].flags &= ~1;
			}
		}
		if ((uVar6 & 0x18) != 0) {
			m_vadstate->Reset();
		}
		auto uVar5 = ProcessDataAndInfo(local_b8, local_98, mat, info);
		if (uVar6 == 1) {
			uVar6 |= uVar5;
		} else {
			if (m_someMatrix.m_rows < 1) {
				if ((uVar6 & 8) != 0) {
					if (field_x30 == 0) {
						uVar6 &= 0xfffffff7;
					} else {
						field_x30 = 0;
						field_xa4 = 2;
					}
					m_someOtherMatrix.Resize(0, 0);
					field_x80.clear();
				}
				uVar6 |= uVar5;
				PrintVlog(static_cast<SnowboySignal>(uVar6), *info);
				return uVar6;
			}
			field_xa0 = uVar6;
			uVar6 = uVar5;
		}
		// TODO: We get here with a empty result way to often, doesnt seem to cause problems but I dont know why
		PrintVlog(static_cast<SnowboySignal>(uVar6), *info);
		return uVar6;
	}

	void VadStateStream::PrintVlog(SnowboySignal, const std::vector<FrameInfo>&) const {
	}

	bool VadStateStream::Reset() {
		m_vadstate->Reset();
		field_xa0 = 2;
		field_xa4 = 1;
		m_someMatrix.Resize(0, 0);
		field_x50.clear();
		m_someOtherMatrix.Resize(0, 0);
		field_x80.clear();
		field_x30 = false;
		return true;
	}

	std::string VadStateStream::Name() const {
		return "VadStateStream";
	}

	VadStateStream::~VadStateStream() {}

} // namespace snowboy
