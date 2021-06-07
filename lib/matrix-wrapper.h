#pragma once
#include <cstdint>
#include <matrix-types.h>
#include <ostream>
#include <string>
#include <vector>

namespace snowboy {
	class VectorBase;
	struct SubMatrix;
	struct MatrixBase {
		size_t m_rows{0};
		size_t m_cols{0};
		size_t m_stride{0};
		float* m_data{nullptr};

		size_t rows() const noexcept { return m_rows; }
		size_t cols() const noexcept { return m_cols; }
		size_t stride() const noexcept { return m_stride; }
		float* data() const noexcept { return m_data; }
		float* data(size_t row) const noexcept { return m_data + (row * stride()); }
		float& operator()(size_t row, size_t col) const noexcept { return m_data[row * m_stride + col]; }
		bool empty() const noexcept { return rows() == 0 || cols() == 0; }

		void AddMat(float alpha, const MatrixBase& A, MatrixTransposeType transA);
		void AddMatMat(float, const MatrixBase&, MatrixTransposeType, const MatrixBase&, MatrixTransposeType, float);
		void AddVecToRows(float, const VectorBase&);
		void AddVecVec(float, const VectorBase&, const VectorBase&);
		void ApplyFloor(float);
		SubMatrix ColRange(size_t, size_t) const;
		void CopyColFromVec(const VectorBase&, size_t);
		void CopyCols(const MatrixBase&, const std::vector<ssize_t>&);
		void CopyColsFromVec(const VectorBase&);
		void CopyDiagFromVec(const VectorBase&);
		void CopyFromMat(const MatrixBase&, MatrixTransposeType transposeType);
		void CopyRowFromVec(const VectorBase&, size_t);
		void CopyRows(const MatrixBase&, const std::vector<ssize_t>&);
		void CopyRowsFromVec(const VectorBase&);
		bool IsDiagonal(float) const;
		bool IsSymmetric(float) const;
		bool IsUnit(float) const;
		// Implementation does a Max and checks that against cutoff
		// We can probably cancel early if the current value is above cutoff
		bool IsZero(float cutoff = 0.00001) const;
		void MulColsVec(const VectorBase&);
		void MulRowsVec(const VectorBase&);
		SubMatrix Range(size_t, size_t, size_t, size_t) const;
		void Read(bool, bool, std::istream*);
		void Read(bool, std::istream*); // Read(p1, false, p2);
		SubMatrix RowRange(size_t, size_t) const;
		void Scale(float factor);
		void Set(float value);
		void SetRandomGaussian();
		void SetRandomUniform();
		void SetUnit();
		void Transpose();
		void Write(bool, std::ostream*) const;
		bool HasNan() const;
		bool HasInfinity() const;
	};
	struct Matrix : MatrixBase {
		Matrix() {}
		Matrix(const Matrix& other) {
			Resize(other.m_rows, other.m_cols, MatrixResizeType::kUndefined);
			CopyFromMat(other, MatrixTransposeType::kNoTrans);
		}
		Matrix(const MatrixBase& other) {
			Resize(other.m_rows, other.m_cols, MatrixResizeType::kUndefined);
			CopyFromMat(other, MatrixTransposeType::kNoTrans);
		}
		Matrix(Matrix&& other) {
			m_rows = other.m_rows;
			m_cols = other.m_cols;
			m_stride = other.m_stride;
			m_data = other.m_data;
			other.m_rows = 0;
			other.m_data = nullptr;
			other.m_stride = 0;
			other.m_cols = 0;
		}
		void Resize(size_t rows, size_t cols, MatrixResizeType resize = MatrixResizeType::kSetZero);
		void AllocateMatrixMemory(size_t rows, size_t cols);
		void ReleaseMatrixMemory(); // NOTE: Called destroy in kaldi
		~Matrix() { ReleaseMatrixMemory(); }

		Matrix& operator=(const Matrix& other);
		Matrix& operator=(const MatrixBase& other);
		Matrix& operator=(Matrix&& other) {
			Swap(&other);
			return *this;
		}

		void RemoveRow(size_t row);
		void Read(bool, bool, std::istream*);
		void Read(bool, std::istream*);
		void Swap(Matrix* other);
		void Transpose();

		static void PrintAllocStats(std::ostream&);
		static void ResetAllocStats();
	};
	struct SubMatrix : MatrixBase {
		SubMatrix(const MatrixBase& parent, size_t rowoffset, size_t rows, size_t coloffset, size_t cols);
	};

	std::ostream& operator<<(std::ostream&, const MatrixBase&);
} // namespace snowboy
