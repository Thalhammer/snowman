#include <cmath>
#include <dtw-lib.h>
#include <limits>
#include <matrix-wrapper.h>
#include <snowboy-error.h>
#include <vector-wrapper.h>

namespace snowboy {

	SlidingDtw::SlidingDtw() {
		m_options.band_width = 20;
		m_options.distance_metric = "euclidean";
		field_x70 = m_options.band_width / 2;
	}

	SlidingDtw::SlidingDtw(const SlidingDtwOptions& opts) {
		SetOptions(opts);
	}

	void SlidingDtw::UpdateDistance(int param_1, const MatrixBase& param_2) {
		// TODO: This seems to generate the right results but I have no fucking clue whats going on
		for (auto iVar25 = param_2.rows() - param_1; iVar25 < param_2.rows(); iVar25++) {
			size_t local_b0 = 0, local_ac = 0;
			ComputeBandBoundary(iVar25, &local_b0, &local_ac);
			std::deque<float> local_88;
			local_88.resize(local_ac - local_b0 + 1);
			for (auto iVar13 = local_b0; iVar13 <= local_ac; iVar13++) {
				auto fVar27 = ComputeVectorDistance(SubVector{*m_reference, iVar13}, SubVector{param_2, iVar25});
				local_88[iVar13 - local_b0] = fVar27;
			}
			field_x18.push_back(local_88);
		}
		auto iVar25 = field_x18.size() - param_2.rows();
		if (iVar25 != 0) {
			if (field_x18.size() > param_2.rows()) {
				field_x18.erase(field_x18.begin(), field_x18.begin() + (field_x18.size() - param_2.rows()));
			}
			for (size_t iVar13 = 0; iVar13 < param_2.rows() - param_1; iVar13++) {
				size_t local_b4 = 0, local_b0 = 0, local_ac = 0, local_a8 = 0;
				ComputeBandBoundary(iVar13, &local_b4, &local_b0);
				ComputeBandBoundary(iVar25 + iVar13, &local_ac, &local_a8);
				if (local_b0 < local_ac) {
					field_x18[iVar13].pop_back();
				} else {
					for (auto i = local_b0 + 1; i <= local_a8; i++) {
						field_x18[iVar13].pop_back();
					}
				}
				// TODO
				size_t i = (local_b0 < local_ac) ? (local_b0 + 1) : (local_ac);
				for (; i >= local_b4 + 1; i--) {
					auto fVar27 = ComputeVectorDistance(SubVector{*m_reference, i - 1}, SubVector{param_2, iVar13});
					field_x18[iVar13].push_front(fVar27);
				}
			}
		}
	}

	void SlidingDtw::SetReference(const MatrixBase* ref) {
		m_reference = ref;
	}

	void SlidingDtw::SetOptions(const SlidingDtwOptions& opts) {
		if (opts.distance_metric == "cosine")
			m_distance_function = DistanceType::cosine;
		else if (opts.distance_metric == "euclidean")
			m_distance_function = DistanceType::euclidean;
		else
			throw snowboy_exception{"Unknown distance type: " + opts.distance_metric};
		m_options = opts;
		field_x70 = m_options.band_width / 2;
	}

	void SlidingDtw::SetEarlyStopThreshold(float t) {
		m_early_stop_threshold = t;
	}

	void SlidingDtw::Reset() {
		field_x18.clear();
	}

	size_t SlidingDtw::GetWindowSize() const {
		if (m_reference) return m_reference->rows();
		return 0;
	}

	float SlidingDtw::GetDistance(int param_1, int param_2) const {
		size_t local_20, local_1c;
		ComputeBandBoundary(param_1, &local_20, &local_1c);
		// Is this correct ?
		return field_x18[param_1][param_2 - local_20];
	}

	float SlidingDtw::ComputeVectorDistance(const VectorBase& param_1, const VectorBase& param_2) const {
		switch (m_distance_function) {
		case DistanceType::cosine: return param_1.CosineDistance(param_2);
		case DistanceType::euclidean: return param_1.EuclideanDistance(param_2);
		default: SNOWBOY_ASSERT(false); return 0;
		}
	}

	float SlidingDtw::ComputeDtwDistance(int param_1, const MatrixBase& param_2) {
		if (m_reference == nullptr)
			throw snowboy_exception{"Reference file has not been set, call SetReference() first!"};
		UpdateDistance(param_1, param_2);

		std::vector<float> local_238;
		auto local_22c = std::numeric_limits<float>::max();
		for (size_t row = 0; row < param_2.rows(); row++) {
			/* try { // try from 00101d18 to 00101d74 has its CatchHandler @ 00102283 */
			size_t local_1e8 = 0, local_1e4 = 0, local_1e0 = 0, local_1dc = 0;
			ComputeBandBoundary(row, &local_1e8, &local_1e4);
			if (0 != row) {
				ComputeBandBoundary(row - 1, &local_1e0, &local_1dc);
			}
			if (local_1e4 < local_1e8) break;
			std::vector<float> __s;
			__s.resize((local_1e4 - local_1e8) + 1);
			auto bVar3 = true;
			for (auto uVar6 = local_1e8; uVar6 <= local_1e4; uVar6++) {
				if (uVar6 == 0 && row == 0) {
					__s[0] = GetDistance(0, 0);
				} else if (row == 0) {
					__s[uVar6 - local_1e8] = GetDistance(0, uVar6) + __s[(uVar6 - 1) - local_1e8];
				} else if (uVar6 == 0) {
					__s[0] = GetDistance(0, 0) + local_238[0];
				} else {
					auto local_240 = std::numeric_limits<float>::max();
					if (local_1e8 < uVar6 && uVar6 - 1 <= local_1e4) {
						local_240 = __s[uVar6 - 1 - local_1e8];
					}
					if (uVar6 >= local_1e0) {
						if (uVar6 <= local_1dc) {
							local_240 = std::min(local_238[uVar6 - local_1e0], local_240);
						}
						if (local_1e0 < uVar6 && uVar6 - 1 <= local_1dc) {
							local_240 = std::min(local_238[uVar6 - 1 - local_1e0], local_240);
						}
					}
					__s[uVar6 - local_1e8] = GetDistance(row, uVar6) + local_240;
				}
				if (bVar3) {
					if (__s[uVar6 - local_1e8] < m_reference->rows() * m_early_stop_threshold) {
						bVar3 = false;
					}
				}
				if (m_reference->rows() - 1 == uVar6 && m_reference->rows() - field_x70 - 1 <= row) {
					local_22c = std::min(__s.back(), local_22c);
					if (local_1e4 < uVar6 + 1) break;
					continue;
				}
			}
			if (bVar3) break;
			local_238 = std::move(__s);
		}
		return local_22c / static_cast<float>(this->m_reference->rows());
	}

	void SlidingDtw::ComputeBandBoundary(int param_1, size_t* param_2, size_t* param_3) const {
		*param_2 = std::max<ssize_t>(param_1 - field_x70, 0);
		*param_3 = std::min<ssize_t>(m_reference->rows() - 1, param_1 + field_x70);
	}

	SlidingDtw::~SlidingDtw() {}

	float DtwAlign(DistanceType param_1, const MatrixBase& param_2, const MatrixBase& param_3, std::vector<std::vector<size_t>>* param_4) {
		if (param_4 != nullptr) param_4->resize(param_2.rows());
		if (param_2.rows() == 0 || param_3.rows() == 0) {
			return std::numeric_limits<float>::max();
		}

		SNOWBOY_ASSERT(!param_2.HasNan() && !param_2.HasInfinity());
		SNOWBOY_ASSERT(!param_3.HasNan() && !param_3.HasInfinity());

		Matrix distances;
		distances.Resize(param_2.rows(), param_3.rows());
		for (size_t row = 0; row < distances.rows(); row++) {
			for (size_t col = 0; col < distances.cols(); col++) {
				if (param_1 == DistanceType::cosine) {
					distances(row, col) = SubVector{param_2, row}.CosineDistance(SubVector{param_3, col});
				} else if (param_1 == DistanceType::euclidean) {
					distances(row, col) = SubVector{param_2, row}.EuclideanDistance(SubVector{param_3, col});
				} else
					throw snowboy_exception{"Unknown distance type: " + std::to_string(param_1)};
			}
		}
		SNOWBOY_ASSERT(!distances.HasNan() && !distances.HasInfinity());
		Matrix local_1d8;
		local_1d8.Resize(param_2.rows(), param_3.rows());
		for (size_t row = 0; row < local_1d8.rows(); row++) {
			for (size_t col = 0; col < local_1d8.cols(); col++) {
				if (row == 0) {
					local_1d8(0, col) = distances(0, col);
				} else if (col == 0) {
					local_1d8(row, 0) = distances(row, 0) + local_1d8(row - 1, 0);
				} else {
					auto fVar16 = std::min(std::min(local_1d8(row, col - 1), local_1d8(row - 1, col)), local_1d8(row - 1, col - 1));
					local_1d8(row, col) = fVar16 + distances(row, col);
				}
			}
		}
		SNOWBOY_ASSERT(!local_1d8.HasNan() && !local_1d8.HasInfinity());
		int min_index = -1;
		auto min_value = SubVector{local_1d8, local_1d8.rows() - 1}.Min(&min_index);
		SNOWBOY_ASSERT(min_index >= 0);
		if (param_4 != nullptr) {
			for (size_t iVar11 = local_1d8.rows() - 1; iVar11 != 0;) {
				// TODO: This is wrong
				// If I look at the code it should only be
				// param_4->at(iVar11).push_back(min_index);
				// But that produces different results from what it should
				if (param_4->at(iVar11).empty())
					param_4->at(iVar11).push_back(min_index);
				else
					param_4->at(iVar11).at(0) = min_index;
				if (0 >= min_index) {
					iVar11--;
					continue;
				}
				auto fVar18 = local_1d8(iVar11, min_index) - distances(iVar11, min_index);
				auto pfVar8_0 = fabs(fVar18 - local_1d8(iVar11 - 1, min_index - 1));
				auto pfVar8_1 = fabs(fVar18 - local_1d8(iVar11, min_index - 1));
				auto pfVar8_2 = fabs(fVar18 - local_1d8(iVar11 - 1, min_index));
				if (pfVar8_0 <= pfVar8_1) {
					if (pfVar8_2 >= pfVar8_0) {
						min_index -= 1;
					}
					iVar11--;
				} else {
					if (pfVar8_2 < pfVar8_1) {
						iVar11--;
					} else {
						min_index -= 1;
					}
				}
			}
			// TODO: This is wrong
			// If I look at the code it should only be
			// param_4->at(0).push_back(min_index);
			// But that produces different results from what it should
			if (param_4->at(0).empty())
				param_4->at(0).push_back(min_index);
			else
				param_4->at(0).at(0) = min_index;
		}
		return min_value / param_2.rows();
	}
} // namespace snowboy
