#pragma once
#include <cstdint>
#include <matrix-types.h>
#include <string>

namespace snowboy {
	struct MatrixBase;
	struct SubVector;
	struct VectorBase {
		uint32_t m_size{0};
		float* m_data{nullptr};

        float* begin() const noexcept { return m_data; }
        float* end() const noexcept { return m_data + m_size; }
        size_t size() const noexcept { return m_size; }
        float* data() const noexcept { return m_data; }
        float& operator[](size_t index) const noexcept { return m_data[index]; }
        bool empty() const noexcept { return size() == 0; }

		void Add(float x);
		void AddDiagMat2(float, const MatrixBase&, MatrixTransposeType, float);
		void AddMatVec(float, const MatrixBase&, MatrixTransposeType, const VectorBase&, float);
		void AddVec(float, const VectorBase&);
		void AddVec2(float, const VectorBase&);
		void ApplyFloor(float);
		void ApplyLog();
		void ApplyPow(float);
		float ApplySoftmax();
		void CopyColsFromMat(const MatrixBase&);
		void CopyFromVec(const VectorBase&);
		void CopyRowsFromMat(const MatrixBase&);
		float CosineDistance(const VectorBase&) const;
		float DotVec(const VectorBase&) const;
		float EuclideanDistance(const VectorBase&) const;
		bool IsZero(float) const;
		float Max() const;
		float Max(int* e) const;
		float Min() const;
		float Min(int* e) const;
		void MulElements(const VectorBase&);
		float Norm(float) const;
		SubVector Range(int, int) const;
		SubVector Range(int, int);
		void Scale(float);
		void Set(float);
		void SetRandomGaussian();
		void SetRandomUniform();
		float Sum() const;
		void Write(bool, std::ostream*) const;
	};
	struct Vector : VectorBase {
		Vector() {}
		Vector(const VectorBase& other) {
			Resize(other.m_size, MatrixResizeType::kUndefined);
			CopyFromVec(other);
		}
		Vector(Vector&& other) {
			m_size = other.m_size;
			m_data = other.m_data;
			other.m_data = nullptr;
			other.m_size = 0;
		}

		void Resize(int size, MatrixResizeType resize = MatrixResizeType::kSetZero);
		~Vector();

		Vector& operator=(const Vector& other);
		Vector& operator=(const VectorBase& other);
		Vector& operator=(Vector&& other) {
			Swap(&other);
			return *this;
		}

		void Read(bool, bool, std::istream*);
		void Read(bool, std::istream*);
		void Swap(Vector* other);
		void RemoveElement(int index);

		static void PrintAllocStats(std::ostream&);
		static void ResetAllocStats();
	};
	struct SubVector : VectorBase {
        // TODO: Those int should be size_t or at least uint
		SubVector(const VectorBase& parent, int, int);
		SubVector(const MatrixBase& parent, int);
		SubVector(const SubVector& other);
	};
} // namespace snowboy