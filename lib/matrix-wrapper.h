#pragma once
#include <cstdint>
#include <matrix-types.h>
#include <ostream>
#include <string>
#include <vector>

namespace snowboy {
	struct VectorBase;
	struct SubMatrix;
	struct MatrixBase {
		uint32_t m_rows{0};
		uint32_t m_cols{0};
		uint32_t m_stride{0};
		float* m_data{nullptr};

		void AddMat(float alpha, const MatrixBase& A, MatrixTransposeType transA);
		void AddMatMat(float, const MatrixBase&, MatrixTransposeType, const MatrixBase&, MatrixTransposeType, float);
		void AddVecToRows(float, const VectorBase&);
		void AddVecVec(float, const VectorBase&, const VectorBase&);
		void ApplyFloor(float);
		SubMatrix ColRange(int, int) const;
		void CopyColFromVec(const VectorBase&, int);
		void CopyCols(const MatrixBase&, const std::vector<int>&);
		void CopyColsFromVec(const VectorBase&);
		void CopyDiagFromVec(const VectorBase&);
		void CopyFromMat(const MatrixBase&, MatrixTransposeType transposeType);
		void CopyRowFromVec(const VectorBase&, int);
		void CopyRows(const MatrixBase&, const std::vector<int>&);
		void CopyRowsFromVec(const VectorBase&);
		bool IsDiagonal(float) const;
		bool IsSymmetric(float) const;
		bool IsUnit(float) const;
		// Implementation does a Max and checks that against cutoff
		// We can probably cancel early if the current value is above cutoff
		bool IsZero(float cutoff = 0.00001) const;
		void MulColsVec(const VectorBase&);
		void MulRowsVec(const VectorBase&);
		SubMatrix Range(int, int, int, int) const;
		void Read(bool, bool, std::istream*);
		void Read(bool, std::istream*); // Read(p1, false, p2);
		SubMatrix RowRange(int, int) const;
		void Scale(float factor);
		void Set(float value);
		void SetRandomGaussian();
		void SetRandomUniform();
		void SetUnit();
		void Transpose();
		void Write(bool, std::ostream*) const;
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
		void Resize(int rows, int cols, MatrixResizeType resize = MatrixResizeType::kSetZero);
		void AllocateMatrixMemory(int rows, int cols);
		void ReleaseMatrixMemory(); // NOTE: Called destroy in kaldi
		~Matrix() { ReleaseMatrixMemory(); }

		Matrix& operator=(const Matrix& other);
		Matrix& operator=(const MatrixBase& other);
		Matrix& operator=(Matrix&& other) {
			ReleaseMatrixMemory();
			m_rows = other.m_rows;
			m_cols = other.m_cols;
			m_stride = other.m_stride;
			m_data = other.m_data;
			other.m_rows = 0;
			other.m_data = nullptr;
			other.m_stride = 0;
			other.m_cols = 0;
			return *this;
		}

		void RemoveRow(int row);
		void Read(bool, bool, std::istream*);
		void Read(bool, std::istream*);
		void Swap(Matrix* other);
		void Transpose();

		static void PrintAllocStats(std::ostream&);
		static void ResetAllocStats();
	};
	struct SubMatrix : MatrixBase {
		SubMatrix(const MatrixBase& parent, int rowoffset, int rows, int coloffset, int cols);
	};

	std::ostream& operator<<(std::ostream&, const MatrixBase&);
} // namespace snowboy