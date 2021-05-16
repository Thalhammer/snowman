#include <dtw-lib.h>
#include <limits>
#include <matrix-wrapper.h>
#include <snowboy-debug.h>
#include <vector-wrapper.h>

namespace snowboy {

	SlidingDtw::SlidingDtw() {
		m_options.band_width = 20;
		m_options.distance_metric = "euclidean";
		field_x70 = m_options.band_width / 2;
	}

	SlidingDtw::SlidingDtw(const SlidingDtwOptions& opts) {
		m_options = opts;
		field_x70 = m_options.band_width / 2;
	}

	void SlidingDtw::UpdateDistance(int param_1, const MatrixBase& param_2) {
		// TODO: This seems to generate the right results but I have no fucking clue whats going on
		for (auto iVar25 = param_2.m_rows - param_1; iVar25 < param_2.m_rows; iVar25++) {
			int local_b0, local_ac;
			ComputeBandBoundary(iVar25, &local_b0, &local_ac);
			std::deque<float> local_88;
			local_88.resize(local_ac - local_b0 + 1);
			for (auto iVar13 = local_b0; iVar13 <= local_ac; iVar13++) {
				auto fVar27 = ComputeVectorDistance(SubVector{*m_reference, iVar13}, SubVector{param_2, iVar25});
				local_88[iVar13 - local_b0] = fVar27;
			}
			field_x18.push_back(local_88);
		}
		auto iVar25 = field_x18.size() - param_2.m_rows;
		if (iVar25 != 0) {
			while (field_x18.size() > param_2.m_rows) {
				field_x18.pop_front();
			}
			for (auto iVar13 = 0; iVar13 < param_2.m_rows - param_1; iVar13++) {
				int local_b4, local_b0, local_ac, local_a8;
				ComputeBandBoundary(iVar13, &local_b4, &local_b0);
				ComputeBandBoundary(iVar25 + iVar13, &local_ac, &local_a8);
				auto iVar14 = local_b0;
				if (local_b0 < local_ac) {
					field_x18[iVar13].pop_back();
				} else {
					for (iVar14 = local_b0 + 1; iVar14 <= local_a8; iVar14++) {
						field_x18[iVar13].pop_back();
					}
					iVar14 = local_ac + -1;
				}
				for (; local_b4 <= iVar14; iVar14--) {
					auto fVar27 = ComputeVectorDistance(SubVector{*m_reference, iVar14}, SubVector{param_2, iVar13});
					field_x18[iVar13].push_front(fVar27);
				}
			}
		}
	}

	void SlidingDtw::SetReference(const MatrixBase* ref) {
		m_reference = ref;
	}

	void SlidingDtw::SetOptions(const SlidingDtwOptions& opts) {
		m_options = opts;
		field_x70 = m_options.band_width / 2;
	}

	void SlidingDtw::SetEarlyStopThreshold(float t) {
		m_early_stop_threshold = t;
	}

	void SlidingDtw::Reset() {
		field_x18.clear();
	}

	int SlidingDtw::GetWindowSize() const {
		if (m_reference) return m_reference->m_rows;
		return 0;
	}

	float SlidingDtw::GetDistance(int param_1, int param_2) const {
		int local_20, local_1c;
		ComputeBandBoundary(param_1, &local_20, &local_1c);
		// Is this correct ?
		return field_x18[param_1][param_2 - local_20];
	}

	float SlidingDtw::ComputeVectorDistance(const VectorBase& param_1, const VectorBase& param_2) const {
		if (m_options.distance_metric == "cosine") {
			return param_1.CosineDistance(param_2);
		} else if (m_options.distance_metric == "euclidean") {
			return param_1.EuclideanDistance(param_2);
		} else {
			SNOWBOY_ERROR() << "Unknown distance type: " << m_options.distance_metric;
			return std::numeric_limits<float>::max();
		}
	}

	float SlidingDtw::ComputeDtwDistance(int param_1, const MatrixBase& param_2) {
		if (m_reference == nullptr) {
			SNOWBOY_ERROR() << "Reference file has not been set, call SetReference() first!";
			return -1;
		}
		UpdateDistance(param_1, param_2);
		if (param_2.m_rows < 1) {
			return std::numeric_limits<float>::max() / static_cast<float>(this->m_reference->m_rows);
		} else {
			std::vector<float> local_238;
			auto local_22c = std::numeric_limits<float>::max();
			for (auto row = 0; row < param_2.m_rows; row++) {
				/* try { // try from 00101d18 to 00101d74 has its CatchHandler @ 00102283 */
				int local_1e8, local_1e4, local_1e0, local_1dc;
				snowboy::SlidingDtw::ComputeBandBoundary(row, &local_1e8, &local_1e4);
				if (0 < row) {
					snowboy::SlidingDtw::ComputeBandBoundary(row - 1, &local_1e0, &local_1dc);
				}
				std::vector<float> __s;
				__s.resize((local_1e4 - local_1e8) + 1);
				if (local_1e4 < local_1e8) break;
				auto bVar3 = true;
				for (auto uVar6 = local_1e8; uVar6 <= local_1e4; uVar6++) {
					if (uVar6 == 0 || row == 0) {
						if (uVar6 == 0 && row == 0) {
							__s[0] = snowboy::SlidingDtw::GetDistance(0, 0);
						} else if (row == 0) {
							__s[uVar6 - local_1e8] = snowboy::SlidingDtw::GetDistance(0, uVar6) + __s[(int)((uVar6 - 1) - local_1e8)];
						} else if (uVar6 == 0) {
							__s[0] = snowboy::SlidingDtw::GetDistance(0, 0) + local_238[0];
						}
					} else {
						auto local_244 = std::numeric_limits<float>::max();
						if (local_1e8 < uVar6 && uVar6 - 1 <= local_1e4) {
							local_244 = __s[uVar6 - 1 - local_1e8];
						}
						float fVar10 = std::numeric_limits<float>::max(), local_240 = std::numeric_limits<float>::max();
						if (uVar6 >= local_1e0) {
							if (uVar6 <= local_1dc) {
								fVar10 = local_238[uVar6 - local_1e0];
							}
							if (local_1e0 < uVar6 && uVar6 - 1 <= local_1dc) {
								local_240 = local_238[uVar6 - 1 - local_1e0];
							}
						}
						auto fVar9 = snowboy::SlidingDtw::GetDistance(row, uVar6);
						local_240 = std::min(fVar10, std::min(local_240, local_244));
						__s[uVar6 - local_1e8] = fVar9 + local_240;
					}
					if (bVar3) {
						if (__s[uVar6 - local_1e8] < m_reference->m_rows * m_early_stop_threshold) {
							bVar3 = false;
						}
					}
					if (m_reference->m_rows - 1 == uVar6 && m_reference->m_rows - field_x70 - 1 <= row) {
						local_22c = std::min(__s.back(), local_22c);
						if (local_1e4 < (int)uVar6 + 1) break;
						continue;
					}
				}
				if (bVar3) break;
				local_238 = __s;
			}
			return local_22c / static_cast<float>(this->m_reference->m_rows);
		}
	}

	void SlidingDtw::ComputeBandBoundary(int param_1, int* param_2, int* param_3) const {
		*param_2 = std::max(param_1 - field_x70, 0);
		auto iVar2 = param_1 + field_x70;
		auto uVar1 = m_reference->m_rows;
		if ((int)uVar1 <= iVar2) {
			iVar2 = uVar1 - 1;
		}
		*param_3 = iVar2;
	}

	SlidingDtw::~SlidingDtw() {}

	float DtwAlign(DistanceType param_1, const MatrixBase& param_2, const MatrixBase& param_3, std::vector<std::vector<int>>* param_4) {
		// TODO: Clean up this function
		if (param_4 != nullptr) param_4->resize(param_2.m_rows);
		if (param_2.m_rows == 0 || param_3.m_rows == 0) {
			return std::numeric_limits<float>::max();
		}

		Matrix local_1f8;
		local_1f8.Resize(param_2.m_rows, param_3.m_rows);
		for (auto row = 0; row < local_1f8.m_rows; row++) {
			for (auto col = 0; col < (int)local_1f8.m_cols; col++) {
				if (param_1 == DistanceType::cosine) {
					auto fVar18 = SubVector{param_2, row}.CosineDistance(SubVector{param_3, col});
					local_1f8.m_data[(row * local_1f8.m_stride) + col] = fVar18;
				} else if (param_1 == DistanceType::euclidean) {
					auto fVar18 = SubVector{param_2, row}.EuclideanDistance(SubVector{param_3, col});
					local_1f8.m_data[(row * local_1f8.m_stride) + col] = fVar18;
				} else {
					SNOWBOY_ERROR() << "Unknown distance type: " << param_1;
				}
			}
		}
		Matrix local_1d8;
		local_1d8.Resize(param_2.m_rows, param_3.m_rows);
		for (auto row = 0; row != local_1d8.m_rows; row++) {
			const int iVar7 = (int(row) - 1) * (int)local_1d8.m_stride;
			const int iVar14 = row * local_1f8.m_stride;
			const int local_284 = row * local_1f8.m_stride;
			const int iVar11 = row * local_1d8.m_stride;
			const int iVar15 = row * local_1d8.m_stride;
			if (0 < local_1d8.m_cols) {
				auto pfVar9 = local_1d8.m_data + (long)iVar7 + -1;
				auto pfVar8 = local_1d8.m_data + (long)iVar11 + -1;
				if (row == 0) {
					auto lVar12 = 0;
					do {
						while (((int)lVar12 == 0 || (row == 0))) {
							pfVar9 = pfVar9 + 1;
							local_1d8.m_data[lVar12] = local_1f8.m_data[lVar12];
							lVar12 += 1;
							pfVar8 = pfVar8 + 1;
							if (local_1d8.m_cols <= (int)lVar12) goto LAB_00186e9d;
						}
						auto fVar16 = pfVar9[1];
						if (*pfVar8 <= pfVar9[1]) {
							fVar16 = *pfVar8;
						}
						if (*pfVar9 <= fVar16) {
							fVar16 = *pfVar9;
						}
						auto lVar6 = iVar14 + lVar12;
						lVar12 += 1;
						pfVar8[1] = fVar16 + local_1f8.m_data[lVar6];
						pfVar9 = pfVar9 + 1;
						pfVar8 = pfVar8 + 1;
					} while ((int)lVar12 < local_1d8.m_cols);
				} else {
					auto lVar12 = 0;
					do {
						if (((int)lVar12 == 0) || (row == 0)) {
							if ((int)lVar12 == 0) {
								local_1d8.m_data[iVar15] = local_1f8.m_data[local_284] + local_1d8.m_data[iVar15 - local_1d8.m_stride];
							}
						} else {
							auto fVar16 = pfVar9[1];
							if (*pfVar8 <= pfVar9[1]) {
								fVar16 = *pfVar8;
							}
							if (*pfVar9 <= fVar16) {
								fVar16 = *pfVar9;
							}
							pfVar8[1] = fVar16 + local_1f8.m_data[iVar14 + lVar12];
						}
						lVar12 += 1;
						pfVar9 = pfVar9 + 1;
						pfVar8 = pfVar8 + 1;
					} while ((int)lVar12 < local_1d8.m_cols);
				}
			}
		LAB_00186e9d:
			[]() {}(); // TODO: This is just here cause for some reason a label directly before the closing bracket does not work
		}
		auto local_228 = -1;
		int iVar11 = local_1d8.m_rows - 1;
		SubVector{local_1d8, iVar11}.Min(&local_228);
		auto fVar16 = local_1d8.m_data[local_1d8.m_stride * iVar11 + local_228];
		if (param_4 != nullptr) {
			while (iVar11 != 0) {
				// TODO: This is wrong
				// If I look at the code it should only be
				// param_4->at(iVar11).push_back(local_228);
				// But that produces different results from what it should
				if (param_4->at(iVar11).empty())
					param_4->at(iVar11).push_back(local_228);
				else
					param_4->at(iVar11).at(0) = local_228;
				if (0 >= local_228) {
					iVar11--;
					continue;
				}
				auto fVar18 = local_1d8.m_data[local_1d8.m_stride * iVar11 + local_228] - local_1f8.m_data[local_1f8.m_stride * iVar11 + local_228];
				float pfVar8[3] = {fVar18, fVar18, fVar18};
				pfVar8[0] = std::abs(fVar18 - local_1d8.m_data[(iVar11 + -1) * local_1d8.m_stride + (local_228 - 1)]);
				pfVar8[1] = std::abs(fVar18 - local_1d8.m_data[(local_228 - 1) + iVar11 * local_1d8.m_stride]);
				pfVar8[2] = std::abs(fVar18 - local_1d8.m_data[(iVar11 + -1) * local_1d8.m_stride + local_228]);
				auto pfVar9 = pfVar8 + 1;
				if (pfVar8[0] <= pfVar8[1]) {
					pfVar9 = pfVar8;
				}
				if (pfVar8[2] < *pfVar9) {
					pfVar9 = pfVar8 + 2;
				}
				auto iVar10 = (int)((long)((long)pfVar9 - (long)pfVar8) >> 2);
				if (iVar10 != 0) {
					if (iVar10 == 1) {
						local_228 -= 1;
					} else {
						if (iVar10 == 2) {
							iVar11 = iVar11 + -1;
						}
					}
				} else {
					local_228 -= 1;
					iVar11 = iVar11 + -1;
				}
			}
			// TODO: This is wrong
			// If I look at the code it should only be
			// param_4->at(0).push_back(local_228);
			// But that produces different results from what it should
			if (param_4->at(0).empty())
				param_4->at(0).push_back(local_228);
			else
				param_4->at(0).at(0) = local_228;
		}
		return fVar16 / param_2.m_rows;
	}
} // namespace snowboy