#pragma once
#include <deque>
#include <string>
#include <vector>

namespace snowboy {
	struct OptionsItf;
	struct Nnet;
	struct VectorBase;
	struct MatrixBase;

	enum DistanceType { cosine = 1,
						euclidean = 2 };
	struct SlidingDtwOptions {
		int band_width;
		// TODO: This could be replaced with enum DistanceType
		std::string distance_metric;
	};
	static_assert(sizeof(SlidingDtwOptions) == 0x10);
	struct SlidingDtw {
		SlidingDtwOptions m_options;
		std::deque<std::deque<float>> field_x18;
		const MatrixBase* m_reference = nullptr;
		int field_x70 = 0;
		float m_early_stop_threshold = 1.0;

		SlidingDtw();
		SlidingDtw(const SlidingDtwOptions&);
		void UpdateDistance(int, const MatrixBase&);
		void SetReference(const MatrixBase*);
		void SetOptions(const SlidingDtwOptions&);
		void SetEarlyStopThreshold(float);
		void Reset();
		int GetWindowSize() const;
		float GetDistance(int, int) const;
		float ComputeVectorDistance(const VectorBase&, const VectorBase&) const;
		float ComputeDtwDistance(int, const MatrixBase&);
		void ComputeBandBoundary(int, int*, int*) const;
		virtual ~SlidingDtw();
	};
	static_assert(sizeof(SlidingDtw) == 0x78);

	void DtwAlign(DistanceType, const MatrixBase&, const MatrixBase&, std::vector<std::vector<int>>*);
} // namespace snowboy