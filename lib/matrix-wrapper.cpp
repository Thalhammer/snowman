extern "C"
{
#include <cblas.h>
}
#include <cstring>
#include <matrix-wrapper.h>
#include <snowboy-debug.h>
#include <snowboy-io.h>
#include <snowboy-utils.h>
#include <vector-wrapper.h>
#include <cmath>

namespace snowboy {

	void MatrixBase::AddMat(float alpha, const MatrixBase& A, MatrixTransposeType transA) {
		SNOWBOY_ERROR() << "Not implemented";
	}

	void MatrixBase::AddMatMat(float param_1, const MatrixBase& param_2, MatrixTransposeType param_3,
							   const MatrixBase& param_4, MatrixTransposeType param_5, float param_6) {
		const int K = param_3 == MatrixTransposeType::kNoTrans ? param_2.m_cols : param_2.m_rows;
		cblas_sgemm(CBLAS_ORDER::CblasRowMajor, static_cast<CBLAS_TRANSPOSE>(param_3), static_cast<CBLAS_TRANSPOSE>(param_5),
					m_rows, m_cols, K, param_1, param_2.m_data, param_2.m_stride, param_4.m_data, param_4.m_stride, param_6, m_data, m_stride);
	}

	void MatrixBase::AddVecToRows(float param_1, const VectorBase& param_2) {
		if (0x40 < m_cols) {
			Vector temp;
			temp.Resize(m_rows, MatrixResizeType::kUndefined);
			temp.Set(1.0f);
			AddVecVec(param_1, temp, param_2);
		} else {
			for (int row = 0; row < m_rows; row++) {
				for (int col = 0; col < m_cols; col++) {
					m_data[row * m_stride + col] += param_1 * param_2.m_data[col];
				}
			}
		}
	}

	void MatrixBase::AddVecVec(float param_1, const VectorBase& param_2, const VectorBase& param_3) {
		cblas_sger(CBLAS_ORDER::CblasRowMajor, param_2.m_size, param_3.m_size, param_1, param_2.m_data,
				   1, param_3.m_data, 1, m_data, m_stride);
	}

	void MatrixBase::ApplyFloor(float f) {
		for (int r = 0; r < m_rows; r++) {
			for (int c = 0; c < m_cols; c++) {
				m_data[r * m_stride + c] = std::max(m_data[r * m_stride + c], f);
			}
		}
	}

	SubMatrix MatrixBase::ColRange(int param_1, int param_2) const {
		return SubMatrix{*this, 0, m_rows, param_1, param_2};
	}

	void MatrixBase::CopyColFromVec(const VectorBase& param_1, int param_2) {
		for (int i = 0; i < m_rows; i++) {
			m_data[m_stride * i + param_2] = param_1.m_data[i];
		}
	}

	void MatrixBase::CopyCols(const MatrixBase& param_1, const std::vector<int>& param_2) {
		for (auto row = 0; row != m_rows; row++) {
			for (auto col = 0; col < m_cols; col++) {
				auto v = 0.0f;
				if (param_2[col] != -1) {
					v = param_1.m_data[param_2[col] + param_1.m_stride * row];
				}
				m_data[m_stride * row + col] = v;
			}
		}
	}

	void MatrixBase::CopyColsFromVec(const VectorBase&) {
		SNOWBOY_ERROR() << "Not implemented";
	}

	void MatrixBase::CopyDiagFromVec(const VectorBase&) {
		SNOWBOY_ERROR() << "Not implemented";
	}

	void MatrixBase::CopyFromMat(const MatrixBase& param_1, MatrixTransposeType param_2) {
		if (&param_1 == this) return;
		if (param_2 == MatrixTransposeType::kTrans) {
			for (auto row = 0; row < m_rows; row++) {
				for (auto col = 0; col < m_cols; col++) {
					m_data[m_stride * row + col] = param_1.m_data[row + col * param_1.m_stride];
				}
			}
		} else {
			for (auto uVar8 = 0; uVar8 < m_rows; uVar8++) {
				SubVector{*this, uVar8}.CopyFromVec(SubVector{param_1, uVar8});
			}
		}
	}

	void MatrixBase::CopyRowFromVec(const VectorBase&, int) {
		SNOWBOY_ERROR() << "Not implemented";
	}

	void MatrixBase::CopyRows(const MatrixBase& param_1, const std::vector<int>& param_2) {
		for (auto row = 0; row < m_rows; row++) {
			while (param_2[row] != -1) {
				memcpy(&m_data[m_stride * row], param_1.m_data + (param_2[row] * param_1.m_stride), m_cols * sizeof(float));
				row++;
				if (row >= this->m_rows) return;
			}
			memset(&m_data[m_stride * row], 0, m_cols * sizeof(float));
		}
	}

	void MatrixBase::CopyRowsFromVec(const VectorBase& param_1) {
		if (m_rows * m_cols == param_1.m_size) {
			if (m_cols == m_stride) {
				memcpy(this->m_data, param_1.m_data, (m_rows * m_cols) * sizeof(float));
			} else {
				for (auto row = 0; row < m_rows; row++) {
					memcpy(&m_data[param_1.m_size * row], &param_1.m_data[m_cols * row], m_cols * sizeof(float));
				}
			}
		} else {
			if (m_cols == param_1.m_size) {
				for (auto row = 0; row < m_rows; row++) {
					memcpy(&m_data[m_stride * row], param_1.m_data, m_cols * sizeof(float));
				}
			} else {
				SNOWBOY_ERROR() << "Vector size should be NumRows() * NumCols() or NumCols(). Vector size is "
								<< param_1.m_size << ", Matrix size is " << m_rows << " x " << m_cols;
			}
		}
	}

	bool MatrixBase::IsDiagonal(float) const {
		SNOWBOY_ERROR() << "Not implemented";
		return false;
	}

	bool MatrixBase::IsSymmetric(float) const {
		SNOWBOY_ERROR() << "Not implemented";
		return false;
	}

	bool MatrixBase::IsUnit(float) const {
		SNOWBOY_ERROR() << "Not implemented";
		return false;
	}

	bool MatrixBase::IsZero(float cutoff) const {
		SNOWBOY_ERROR() << "Not implemented";
		return false;
	}

	void MatrixBase::MulColsVec(const VectorBase& param_1) {
		for (auto col = 0; col < m_cols; col++) {
			for (auto row = 0; row != m_rows; row++) {
				m_data[col + row * m_stride] *= param_1.m_data[col];
			}
		}
	}

	void MatrixBase::MulRowsVec(const VectorBase& param_1) {
		for (int i = 0; i < m_rows; i++) {
			auto this_scale = param_1.m_data[i];
			for (int j = 0; j < m_cols; j++) {
				m_data[i * m_stride + j] *= this_scale;
			}
		}
	}

	SubMatrix MatrixBase::Range(int param_1, int param_2, int param_3, int param_4) const {
		return SubMatrix{*this, param_1, param_2, param_3, param_4};
	}

	void MatrixBase::Read(bool binary, bool add, std::istream* is) {
		Matrix temp;
		temp.Resize(m_rows, m_cols, MatrixResizeType::kUndefined);
		temp.Read(binary, is);
		if (m_rows != temp.m_rows || m_cols != temp.m_cols) {
			SNOWBOY_ERROR() << "Failed to read Matrix: size mismatch " << m_rows << " x " << m_cols << " v.s. "
							<< temp.m_rows << " x " << temp.m_cols;
			return;
		}
		if (add) {
			AddMat(1.0f, temp, MatrixTransposeType::kNoTrans);
		} else
			CopyFromMat(temp, MatrixTransposeType::kNoTrans);
	}

	void MatrixBase::Read(bool binary, std::istream* is) {
		Read(binary, false, is);
	}

	SubMatrix MatrixBase::RowRange(int param_1, int param_2) const {
		return SubMatrix{*this, param_1, param_2, 0, m_cols};
	}

	void MatrixBase::Scale(float param_1) {
		if (param_1 == 1.0f || m_rows == 0 || m_cols == 0) return;
		if (m_cols == this->m_stride) {
			cblas_sscal(m_cols * m_rows, param_1, m_data, 1);
		} else {
			for (auto r = 0; r < m_rows; r++) {
				cblas_sscal(m_cols, param_1, &m_data[r * m_stride], 1);
			}
		}
	}

	void MatrixBase::Set(float value) {
		for (int r = 0; r < m_rows; r++) {
			for (int c = 0; c < m_cols; c++) {
				m_data[r * m_stride + c] = value;
			}
		}
	}

	void MatrixBase::SetRandomGaussian() {
		SNOWBOY_ERROR() << "Not implemented";
	}

	void MatrixBase::SetRandomUniform() {
		SNOWBOY_ERROR() << "Not implemented";
	}

	void MatrixBase::SetUnit() {
		SNOWBOY_ERROR() << "Not implemented";
	}

	void MatrixBase::Transpose() {
		SNOWBOY_ERROR() << "Not implemented";
	}

	void MatrixBase::Write(bool binary, std::ostream* os) const {
		if (!binary) SNOWBOY_ERROR() << "Not implemented";
		WriteToken(binary, "FM", os);
		WriteBasicType<int32_t>(binary, m_rows, os);
		WriteBasicType<int32_t>(binary, m_cols, os);
		if (m_cols == m_stride) {
			os->write(reinterpret_cast<const char*>(m_data), m_rows * m_cols * sizeof(float));
		} else {
			for (int r = 0; r < m_rows; r++) {
				os->write(reinterpret_cast<const char*>(&m_data[r * m_stride]), m_cols * sizeof(float));
			}
		}
		if (!*os) {
			SNOWBOY_ERROR() << "Fail to write Matrix to stream.";
		}
	}

	bool MatrixBase::HasNan() const {
		for(size_t r = 0; r<rows(); r++) {
			for(size_t c = 0; c<cols(); c++) {
				if((*this)(r,c) != (*this)(r,c)) return true;
			}
		}
		return false;
	}

	bool MatrixBase::HasInfinity() const {
		for(size_t r = 0; r<rows(); r++) {
			for(size_t c = 0; c<cols(); c++) {
				if(std::isinf((*this)(r,c))) return true;
			}
		}
		return false;
	}

	static size_t allocs = 0;
	static size_t frees = 0;

	template <typename T>
	constexpr inline T next_multiple_of(T val, T multi) noexcept {
		return (val + multi - 1) & ~(multi - 1);
	}

	void Matrix::Resize(int rows, int cols, MatrixResizeType resize) {
		if (cols == 0 && rows == 0) {
			m_rows = 0;
			m_cols = 0;
			return;
		}
		// TODO: Smarter alloc similar to vector
		uint64_t mem_size = static_cast<uint64_t>(m_rows) * static_cast<uint64_t>(m_stride);
		uint64_t new_size = static_cast<uint64_t>(rows) * next_multiple_of<uint64_t>(cols, 4);
		if (new_size <= mem_size) {
			if (resize == MatrixResizeType::kUndefined || resize == MatrixResizeType::kSetZero) {
				m_rows = rows;
				m_cols = cols;
				m_stride = next_multiple_of<uint64_t>(cols, 4);
				if (resize == MatrixResizeType::kSetZero) Set(0.0f);
				return;
			} else if (cols <= m_stride) {
				m_rows = rows;
				m_cols = cols;
				return;
			}
		}
		if (m_data == nullptr) {
			AllocateMatrixMemory(rows, cols);
			if (resize == MatrixResizeType::kSetZero) Set(0.0f);
			return;
		}
		if (resize == MatrixResizeType::kCopyData) {
			Matrix temp;
			temp.Resize(rows, cols, MatrixResizeType::kSetZero);
			for (int r = 0; r < std::min(rows, (int)m_rows); r++) {
				memcpy(&temp.m_data[r * temp.m_stride], &m_data[r * m_stride], std::min((int)m_cols, cols) * sizeof(float));
			}
			temp.Swap(this);
		} else {
			ReleaseMatrixMemory();
			AllocateMatrixMemory(rows, cols);
			if (resize == MatrixResizeType::kSetZero) Set(0.0f);
		}
	}

	void Matrix::AllocateMatrixMemory(int rows, int cols) {
		if (rows == 0 || cols == 0) {
			m_data = nullptr;
			m_stride = 0;
			m_rows = 0;
			m_cols = 0;
		}
		m_rows = rows;
		m_cols = cols;
		m_stride = (cols + 3) & ~3;
		SNOWBOY_ASSERT(m_stride % 4 == 0);
		m_data = static_cast<float*>(SnowboyMemalign(16, rows * m_stride * sizeof(float)));
		if (m_data == nullptr) {
			m_stride = 0;
			m_rows = 0;
			m_cols = 0;
			throw std::bad_alloc();
		}
		allocs++;
	}

	void Matrix::ReleaseMatrixMemory() {
		if (m_data) {
			SnowboyMemalignFree(m_data);
			frees++;
		}
		m_rows = 0;
		m_stride = 0;
		m_cols = 0;
	}

	void Matrix::PrintAllocStats(std::ostream& out) {
		out << "allocs=" << allocs << " frees=" << frees;
	}

	void Matrix::ResetAllocStats() {
		allocs = 0;
		frees = 0;
	}

	Matrix& Matrix::operator=(const Matrix& other) {
		Resize(other.m_rows, other.m_cols, MatrixResizeType::kUndefined);
		CopyFromMat(other, MatrixTransposeType::kNoTrans);
		return *this;
	}

	Matrix& Matrix::operator=(const MatrixBase& other) {
		Resize(other.m_rows, other.m_cols, MatrixResizeType::kUndefined);
		CopyFromMat(other, MatrixTransposeType::kNoTrans);
		return *this;
	}

	void Matrix::RemoveRow(int row) {
		// TODO: This could be a memmove
		for (auto r = row + 1; r < (int)m_rows; r++) {
			SubVector{*this, r - 1}.CopyFromVec(SubVector{*this, r});
		}
		m_rows -= 1;
	}

	void Matrix::Read(bool binary, bool add, std::istream* is) {
		if (!binary) {
			SNOWBOY_ERROR() << "Not implemented";
		}
		if (add) {
			Matrix temp;
			temp.Read(binary, false, is);
			if (m_rows == 0)
				Resize(temp.m_rows, temp.m_cols);
			else if (m_rows != temp.m_rows) {
				SNOWBOY_ERROR() << "Fail to read Matrix: size mismatch " << temp.m_rows << " x " << temp.m_cols << " v.s. " << m_rows << " x " << m_cols;
				return;
			}
			AddMat(1.0f, temp, MatrixTransposeType::kNoTrans);
		} else {
			ExpectToken(binary, "FM", is);
			int rows, cols;
			ReadBasicType<int32_t>(binary, &rows, is);
			ReadBasicType<int32_t>(binary, &cols, is);
			if (m_rows != rows || m_cols != cols) {
				Resize(rows, cols, MatrixResizeType::kUndefined);
			}
			if (m_stride == m_cols) {
				is->read(reinterpret_cast<char*>(m_data), m_rows * m_cols * sizeof(float));
			} else {
				for (int r = 0; r < m_rows; r++) {
					is->read(reinterpret_cast<char*>(&m_data[r * m_stride]), m_cols * sizeof(float));
				}
			}
		}
		if (!*is) {
			SNOWBOY_ERROR() << "Fail to read Matrix.";
		}
	}

	void Matrix::Read(bool binary, std::istream* is) {
		Read(binary, false, is);
	}

	void Matrix::Swap(Matrix* other) {
		std::swap(m_cols, other->m_cols);
		std::swap(m_rows, other->m_rows);
		std::swap(m_stride, other->m_stride);
		std::swap(m_data, other->m_data);
	}

	void Matrix::Transpose() {
		if (m_rows != m_cols) {
			Matrix temp;
			temp.Resize(m_cols, m_rows, MatrixResizeType::kUndefined);
			temp.CopyFromMat(*this, MatrixTransposeType::kTrans);
			Resize(m_cols, m_rows, MatrixResizeType::kUndefined);
			CopyFromMat(temp, MatrixTransposeType::kNoTrans);
		} else {
			MatrixBase::Transpose();
		}
	}

	SubMatrix::SubMatrix(const MatrixBase& parent, int rowoffset, int rows, int coloffset, int cols) {
		m_stride = parent.m_stride;
		m_rows = rows;
		m_cols = cols;
		m_data = parent.m_data + (rowoffset * parent.m_stride) + coloffset;
	}

	std::ostream& operator<<(std::ostream& s, const MatrixBase& o) {
		auto flags = s.flags();
		s << std::fixed;
		s.precision(3);
		s << "mat<" << o.m_rows << "," << o.m_cols << ">{\n";
		for (int r = 0; r < o.m_rows; r++) {
			s << "{ ";
			for (int c = 0; c < o.m_cols; c++)
				s << o.m_data[r * o.m_stride + c] << " ";
			s << " }\n";
		}
		s << "}";
		s.flags(flags);
		return s;
	}

} // namespace snowboy
