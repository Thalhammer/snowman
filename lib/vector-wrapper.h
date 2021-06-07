#pragma once
#include <cstdint>
#include <matrix-types.h>
#include <snowboy-debug.h>
#include <stdexcept>
#include <string>

namespace snowboy {
	struct MatrixBase;
	class SubVector;
	class VectorBase {
	protected:
		size_t m_size{0};
		float* m_data{nullptr};

	public:
		float* begin() const noexcept { return m_data; }
		float* end() const noexcept { return m_data + m_size; }
		size_t size() const noexcept { return m_size; }
		float* data() const noexcept { return m_data; }
		float& operator[](size_t index) const noexcept {
			SNOWBOY_ASSERT(index < m_size);
			return m_data[index];
		}
		float& operator()(size_t index) const noexcept {
			SNOWBOY_ASSERT(index < m_size);
			return m_data[index];
		}
		bool empty() const noexcept { return size() == 0; }

		void Add(float x) noexcept;
		void AddDiagMat2(float, const MatrixBase&, MatrixTransposeType, float) noexcept;
		void AddMatVec(float alpha, const MatrixBase& a, MatrixTransposeType trans, const VectorBase& x, float beta) noexcept;
		void AddVec(float, const VectorBase&) noexcept;
		void AddVec2(float, const VectorBase&) noexcept;
		void ApplyFloor(float) noexcept;
		void ApplyLog() noexcept;
		void ApplyPow(float) noexcept;
		float ApplySoftmax() noexcept;
		void CopyColsFromMat(const MatrixBase&) noexcept;
		void CopyFromVec(const VectorBase&) noexcept;
		void CopyRowsFromMat(const MatrixBase&) noexcept;
		float CosineDistance(const VectorBase&) const;
		float DotVec(const VectorBase&) const;
		float EuclideanDistance(const VectorBase&) const;
		bool IsZero(float cutoff) const noexcept;
		float Max() const noexcept;
		float Max(int* e) const noexcept;
		float Min() const noexcept;
		float Min(int* e) const noexcept;
		void MulElements(const VectorBase&) noexcept;
		float Norm(float) const noexcept;
		SubVector Range(size_t offset, size_t size) const noexcept;
		void Scale(float) noexcept;
		void Set(float) noexcept;
		void SetRandomGaussian();
		void SetRandomUniform();
		float Sum() const noexcept;
		void Write(bool, std::ostream*) const;

		bool HasNan() const noexcept;
		bool HasInfinity() const noexcept;
	};
	class Vector : public VectorBase {
	protected:
		size_t m_cap{0};

	public:
		Vector() noexcept {}
		Vector(const VectorBase& other) {
			Resize(other.size(), MatrixResizeType::kUndefined);
			CopyFromVec(other);
		}
		Vector(const Vector& other) {
			Resize(other.m_size, MatrixResizeType::kUndefined);
			CopyFromVec(other);
		}
		Vector(Vector&& other) noexcept {
			m_size = other.m_size;
			m_cap = other.m_cap;
			m_data = other.m_data;
			other.m_data = nullptr;
			other.m_size = 0;
			other.m_cap = 0;
		}

		size_t capacity() const noexcept { return m_cap; }
		void Resize(size_t size, MatrixResizeType resize = MatrixResizeType::kSetZero);
		~Vector() noexcept;

		Vector& operator=(const Vector& other);
		Vector& operator=(const VectorBase& other);
		Vector& operator=(Vector&& other) noexcept {
			Swap(&other);
			return *this;
		}

		void Read(bool, bool, std::istream*);
		void Read(bool, std::istream*);
		void Swap(Vector* other) noexcept;
		void RemoveElement(size_t index) noexcept;

		static void PrintAllocStats(std::ostream&);
		static void ResetAllocStats();
	};
	class SubVector : public VectorBase {
	public:
		SubVector(const VectorBase& parent, size_t offset, size_t size) noexcept;
		SubVector(const MatrixBase& parent, size_t row) noexcept;
		SubVector(const SubVector& other) noexcept {
			m_data = other.m_data;
			m_size = other.m_size;
		}
	};
	template <size_t N>
	class FixedVector : public VectorBase {
		float m_storage[N];

	public:
		FixedVector() noexcept {
			m_data = m_storage;
		}
		FixedVector(const VectorBase& other) {
			m_data = m_storage;
			Resize(other.size(), MatrixResizeType::kUndefined);
			CopyFromVec(other);
		}
		FixedVector(const FixedVector& other) {
			m_data = m_storage;
			Resize(other.m_size, MatrixResizeType::kUndefined);
			CopyFromVec(other);
		}
		FixedVector(size_t size, MatrixResizeType resize = MatrixResizeType::kSetZero) {
			m_data = m_storage;
			Resize(size, resize);
		}

		constexpr size_t capacity() const noexcept { return N; }
		void Resize(size_t size, MatrixResizeType resize = MatrixResizeType::kSetZero) {
			if (size > N) throw std::invalid_argument("new size exceeds fixed capacity");
			m_size = size;
			if (resize == MatrixResizeType::kSetZero) Set(0.0f);
		}

		FixedVector& operator=(const FixedVector& other) {
			Resize(other.m_size, MatrixResizeType::kUndefined);
			CopyFromVec(other);
			return *this;
		}
		FixedVector& operator=(const VectorBase& other) {
			Resize(other.size(), MatrixResizeType::kUndefined);
			CopyFromVec(other);
			return *this;
		}
	};
} // namespace snowboy
