extern "C" {
#include <cblas.h>
}
#include <cmath>
#include <cstring>
#include <limits>
#include <matrix-wrapper.h>
#include <random>
#include <snowboy-debug.h>
#include <snowboy-io.h>
#include <vector-wrapper.h>
#include <iostream>

// TODO: This should be detected by cmake instead of just assuming linux has it
#ifdef __linux__
#include <malloc.h>
#define HAS_MALLOC_USABLE_SIZE 1
#else
#define HAS_MALLOC_USABLE_SIZE 0
#endif

#define DUMP() std::cout << __FUNCTION__ << std::endl;

namespace snowboy {
	void VectorBase::Add(float x) {
		for (int i = 0; i < m_size; i++) {
			m_data[i] += x;
		}
	}

	void VectorBase::AddDiagMat2(float param_1, const MatrixBase& param_2, MatrixTransposeType param_3, float param_4) {
		if (param_3 == MatrixTransposeType::kNoTrans) {
            auto ptr = param_2.m_data;
			for (int i = 0; i < m_size; i++) {
				auto fVar1 = m_data[i];
				auto fVar7 = cblas_sdot(param_2.m_cols, ptr, 1, ptr, 1);
				m_data[i] = fVar7 * param_1 + param_4 * fVar1;
                ptr += param_2.m_stride;
			}
		} else {
			for (int i = 0; i < m_size; i++) {
				auto fVar1 = m_data[i];
				auto fVar7 = cblas_sdot(param_2.m_rows, &param_2.m_data[i], param_2.m_stride, &param_2.m_data[i], param_2.m_stride);
				m_data[i] = fVar7 * param_1 + param_4 * fVar1;
			}
		}
	}

	void VectorBase::AddMatVec(float param_1, const MatrixBase& param_2, MatrixTransposeType param_3, const VectorBase& param_4, float param_5) {
		cblas_sgemv(CBLAS_ORDER::CblasRowMajor, static_cast<CBLAS_TRANSPOSE>(param_3),
					param_2.m_rows, param_2.m_cols, param_1, param_2.m_data, param_2.m_stride, param_4.m_data, 1, param_5, m_data, 1);
	}

	void VectorBase::AddVec(float param_1, const VectorBase& param_2) {
		SNOWBOY_ASSERT(param_2.m_size == m_size);
		cblas_saxpy(m_size, param_1, param_2.m_data, 1, m_data, 1);
	}

	void VectorBase::AddVec2(float param_1, const VectorBase& param_2) {
		SNOWBOY_ASSERT(m_size == param_2.m_size);
		for (size_t i = 0; i < m_size; i++)
			m_data[i] += param_1 * param_2.m_data[i] * param_2.m_data[i];
	}

	void VectorBase::ApplyFloor(float param_1) {
		for (size_t i = 0; i < m_size; i++) {
			m_data[i] = std::max(param_1, m_data[i]);
		}
	}

	void VectorBase::ApplyLog() {
		for (size_t i = 0; i < m_size; i++) {
			m_data[i] = logf(m_data[i]);
		}
	}

	void VectorBase::ApplyPow(float param_1) {
		for (size_t i = 0; i < m_size; i++) {
			m_data[i] = pow(m_data[i], param_1);
		}
	}

	float VectorBase::ApplySoftmax() {
		auto max = Max(), sum = 0.0f;
		for (size_t i = 0; i < m_size; i++) {
			m_data[i] = expf(m_data[i] - max);
			sum += m_data[i];
		}
		Scale(1.0f / sum);
		return logf(sum) + max;
	}

	void VectorBase::CopyColsFromMat(const MatrixBase& param_1) {
		SNOWBOY_ASSERT(m_size >= param_1.m_rows * param_1.m_cols);
		for (auto r = 0; r < param_1.m_rows; r += 1) {
			for (auto c = 0; c < param_1.m_cols; c += 1) {
				m_data[r + param_1.m_rows * c] = param_1.m_data[param_1.m_stride * r + c];
			}
		}
	}

	void VectorBase::CopyFromVec(const VectorBase& param_1) {
		if (m_data != param_1.m_data) {
			memcpy(m_data, param_1.m_data, std::min(m_size, param_1.m_size) * sizeof(float));
		}
	}

	void VectorBase::CopyRowsFromMat(const MatrixBase& param_1) {
		SNOWBOY_ASSERT(m_size >= param_1.m_cols * param_1.m_rows);
		if (param_1.m_cols != param_1.m_stride) {
			for (auto r = 0; r != param_1.m_rows; r++) {
				memcpy(&m_data[r * param_1.m_cols], &param_1.m_data[param_1.m_stride * r], param_1.m_cols * 4);
			}
		} else {
			memcpy(m_data, param_1.m_data, param_1.m_cols * param_1.m_rows * 4);
		}
	}

	float VectorBase::CosineDistance(const VectorBase& param_1) const {
		return (1.0f - (DotVec(param_1) / Norm(2.0f)) / param_1.Norm(2.0f)) * 0.5f;
	}

	float VectorBase::DotVec(const VectorBase& param_1) const {
		return cblas_sdot(std::min(m_size, param_1.m_size), m_data, 1, param_1.m_data, 1);
	}

	float VectorBase::EuclideanDistance(const VectorBase& param_1) const {
		auto sum = 0.0;
		for (uint32_t i = 0; i < std::min(m_size, param_1.m_size); i++) {
			auto fVar2 = m_data[i] - param_1.m_data[i];
			sum += fVar2 * fVar2;
		}
		return sqrtf(sum);
	}

	bool VectorBase::IsZero(float cutoff) const {
		auto max = 0.0f;
		for (uint32_t i = 0; i < m_size; i++) {
			max = std::max(std::abs(m_data[i]), max);
		}
		return max <= cutoff;
	}

	float VectorBase::Max() const {
		auto max = -std::numeric_limits<float>::infinity();
		for (uint32_t i = 0; i < m_size; i++) {
			max = std::max(m_data[i], max);
		}
		return max;
	}

	float VectorBase::Max(int* e) const {
		*e = -1;
		auto max = -std::numeric_limits<float>::infinity();
		for (uint32_t i = 0; i < m_size; i++) {
			if (m_data[i] > max) {
				*e = i;
				max = m_data[i];
			}
		}
		return max;
	}

	float VectorBase::Min() const {
		auto min = std::numeric_limits<float>::infinity();
		for (uint32_t i = 0; i < m_size; i++) {
			min = std::min(m_data[i], min);
		}
		return min;
	}

	float VectorBase::Min(int* e) const {
		*e = -1;
		auto min = std::numeric_limits<float>::infinity();
		for (uint32_t i = 0; i < m_size; i++) {
			if (m_data[i] < min) {
				*e = i;
				min = m_data[i];
			}
		}
		return min;
	}

	void VectorBase::MulElements(const VectorBase& param_1) {
		for (uint32_t i = 0; i < std::min(m_size, param_1.m_size); i++) {
			m_data[i] *= param_1.m_data[i];
		}
	}

	float VectorBase::Norm(float p) const {
		// TODO: Float equal compare is bad
		if (p == 0.0f) {
			float sum = 0.0f;
			for (uint32_t i = 0; i < m_size; i++) {
				if (m_data[i] != 0.0f) sum += 1.0f;
			}
			return sum;
		} else if (p == 1.0f) {
			float sum = 0.0f;
			for (uint32_t i = 0; i < m_size; i++) {
				sum += std::abs(m_data[i]);
			}
			return sum;
		} else if (p == 2.0f) {
			float sum = 0.0f;
			for (uint32_t i = 0; i < m_size; i++) {
				sum += m_data[i] * m_data[i];
			}
			return sum;
		} else {
			float tmp = 0.0f, sum = 0.0f;
			bool ok = true;
			for (uint32_t i = 0; i < m_size; i++) {
				tmp = pow(std::abs(m_data[i]), p);
				if (tmp == HUGE_VAL) ok = false;
				sum += tmp;
			}
			tmp = pow(sum, static_cast<float>(1.0 / p));
			SNOWBOY_ASSERT(tmp != HUGE_VAL);
			if (ok) {
				return tmp;
			} else {
				// TODO: Instead of copying we could scale it on the fly. This should save a allocation/deallocation which tends to be expensive
				float maximum = Max(), minimum = Min(), max_abs = std::max(maximum, -minimum);
				SNOWBOY_ASSERT(max_abs > 0);
				Vector tmp(*this);
				tmp.Scale(1.0 / max_abs);
				return tmp.Norm(p) * max_abs;
			}
		}
	}

	SubVector VectorBase::Range(int param_1, int param_2) const {
		return SubVector(*this, param_1, param_2);
	}

	// TODO: Why do we need this one if the result is the same ?
	SubVector VectorBase::Range(int param_1, int param_2) {
		return SubVector(*this, param_1, param_2);
	}

	void VectorBase::Read(bool binary, bool add, std::istream* is) {
		// TODO: Since reading is always the same, couldn't we just drop it in here ?
		Vector tmp;
		tmp.Resize(m_size, MatrixResizeType::kSetZero);
		tmp.Read(binary, false, is);
		if (tmp.m_size != m_size) {
			SNOWBOY_ERROR() << "Failed to read Vector: size missmatch (" << tmp.m_size << " v.s. " << m_size << ").";
			return;
		}
		if (add) {
			AddVec(1.0f, tmp);
		} else {
			CopyFromVec(tmp);
		}
	}

	void VectorBase::Read(bool binary, std::istream* is) {
		Read(binary, false, is);
	}

	void VectorBase::Scale(float factor) {
		cblas_sscal(m_size, factor, m_data, 1);
	}

	void VectorBase::Set(float val) {
		for (uint32_t i = 0; i < m_size; i++) {
			m_data[i] = val;
		}
	}

	void VectorBase::SetRandomGaussian() {
		SNOWBOY_ERROR() << "Not implemented";
	}

	void VectorBase::SetRandomUniform() {
		SNOWBOY_ERROR() << "Not implemented";
	}

	float VectorBase::Sum() const {
		auto sum = 0.0f;
		for (uint32_t i = 0; i < m_size; i++)
			sum += m_data[i];
		return sum;
	}

	void VectorBase::Write(bool binary, std::ostream* os) const {
		if (!*os) SNOWBOY_ERROR() << "Failed to write Vector to stream.";
		if (!binary) {
			*os << " [ ";
			for (uint32_t i = 0; i < m_size; i++) {
				*os << m_data[i] << " ";
			}
			*os << "]\n";
		} else {
			WriteToken(binary, "FV", os);
			WriteBasicType<int>(binary, m_size, os);
			os->write(reinterpret_cast<const char*>(m_data), m_size * sizeof(float));
		}
		if (!*os) SNOWBOY_ERROR() << "Failed to write Vector to stream.";
	}

    static size_t allocs = 0;
    static size_t frees = 0;
	void Vector::Resize(int size, MatrixResizeType resize) {
		if (size <= m_size) {
			m_size = size;
			if (resize == MatrixResizeType::kSetZero) Set(0.0f);
		} else {
            // The new size is larger than we currently are, so we need to reallocate.
#if HAS_MALLOC_USABLE_SIZE
			auto usable = malloc_usable_size(m_data);
			if (usable >= size * sizeof(float)) {
				if (resize == MatrixResizeType::kSetZero) {
					memset(&m_data[m_size], 0, usable - m_size * sizeof(float));
				}
				m_size = size;
				return;
			}
#endif
			// We dont have usable size or the allocated block was to small
			if (resize == MatrixResizeType::kCopyData) {
				// Since we would copy it anyway we can just call realloc and maybe save copying (e.g. if the next block is free).
				auto ptr = static_cast<float*>(realloc(m_data, size * sizeof(float)));
				if (ptr == nullptr) throw std::bad_alloc();
				if (ptr != m_data && (reinterpret_cast<uintptr_t>(ptr) % 16) != 0) {
					// realloc moved the data but the new buffer is not aligned correctly
                    allocs++;
                    frees++;
					free(ptr);
                    allocs++;
					ptr = static_cast<float*>(SnowboyMemalign(16, size * sizeof(float)));
					if (ptr == nullptr) throw std::bad_alloc();
					memcpy(ptr, m_data, sizeof(float) * m_size);
				}
				m_data = ptr;
				memset(&m_data[m_size], 0, (size - m_size) * sizeof(float));
			} else if (resize == MatrixResizeType::kSetZero) {
                allocs++;
				auto ptr = static_cast<float*>(SnowboyMemalign(16, size * sizeof(float)));
				if (ptr == nullptr) throw std::bad_alloc();
                if(m_data) {
                    frees++;
				    free(m_data);
                }
				memset(ptr, 0, size * sizeof(float));
				m_data = ptr;
			} else {
                allocs++;
				auto ptr = static_cast<float*>(SnowboyMemalign(16, size * sizeof(float)));
				if (ptr == nullptr) throw std::bad_alloc();
                if(m_data) {
                    frees++;
				    free(m_data);
                }
				m_data = ptr;
			}
			m_size = size;
		}
	}

	void Vector::AllocateVectorMemory(int size) {
		if (size == 0) {
			m_data = nullptr;
		} else {
			m_data = static_cast<float*>(SnowboyMemalign(16, size << 2));
			if (m_data == nullptr) throw std::bad_alloc();
            allocs++;
		}
		m_size = size;
	}

	void Vector::ReleaseVectorMemory() {
		if (m_data) {
			SnowboyMemalignFree(m_data);
            frees++;
        }
		m_data = nullptr;
		m_size = 0;
	}

	Vector& Vector::operator=(const Vector& other) {
		Resize(other.m_size, MatrixResizeType::kUndefined);
		CopyFromVec(other);
		return *this;
	}

	Vector& Vector::operator=(const VectorBase& other) {
		Resize(other.m_size, MatrixResizeType::kUndefined);
		CopyFromVec(other);
		return *this;
	}

	void Vector::Read(bool binary, bool add, std::istream* is) {
		if (!binary) {
            SNOWBOY_ERROR() << "Not implemented";
			ExpectToken(binary, "[", is);
			uint32_t i = 0;
			auto s = m_size;
			for (; i < m_size; i++) {
				float f = 0.0f;
				if (!isspace(is->get())) {
					SNOWBOY_ERROR() << "Expecting space after number";
					return;
				}
				if (is->peek() == ']') {
					Resize(i, MatrixResizeType::kCopyData);
					break;
				}
				*is >> f;
				if (add)
					m_data[i] += f;
				else
					m_data[i] = f;
			}
			if (i == s) {
				if (!isspace(is->get())) {
					SNOWBOY_ERROR() << "Expecting space after numbers";
					return;
				}
				if (is->get() != ']') {
					SNOWBOY_ERROR() << "Expecting closing bracket after data";
					return;
				}
			}
			if (is->get() != '\n') {
				SNOWBOY_ERROR() << "Expecting newline after data";
				return;
			}
		} else {
			ExpectToken(binary, "FV", is);
			int size;
			ReadBasicType<int>(binary, &size, is);
			if (!add) {
				Resize(size, MatrixResizeType::kUndefined);
				if (size != 0) {
					is->read(reinterpret_cast<char*>(m_data), size * sizeof(float));
				}
			} else {
				Vector temp;
				temp.Resize(size, MatrixResizeType::kUndefined);
				if (size != 0) {
					is->read(reinterpret_cast<char*>(temp.m_data), size * sizeof(float));
				}
				AddVec(1.0f, temp);
			}
		}
	}

	void Vector::Read(bool binary, std::istream* is) {
		Read(binary, false, is);
	}

	void Vector::Swap(Vector* other) {
		auto tdata = other->m_data;
		other->m_data = m_data;
		m_data = tdata;
		auto tsize = other->m_size;
		other->m_size = m_size;
		m_size = tsize;
	}

	void Vector::RemoveElement(int index) {
		if (index >= m_size) return;
		if (index < m_size - 1) {
			memmove(&m_data[index], &m_data[index + 1], (m_size - index - 1) * sizeof(float));
		}
		m_size--;
	}

    void Vector::PrintAllocStats(std::ostream& out) {
        out << "allocs=" << allocs << " frees=" << frees;
    }

    void Vector::ResetAllocStats() {
        allocs = 0;
        frees = 0;
    }

	SubVector::SubVector(const VectorBase& parent, int offset, int size) {
		m_data = parent.m_data + offset;
		m_size = size; // TODO: std::min<int>(parent.m_size - offset, size);
	}

	SubVector::SubVector(const MatrixBase& parent, int row) {
		m_data = parent.m_data + (row * parent.m_stride);
		m_size = parent.m_cols;
	}

	SubVector::SubVector(const SubVector& other) {
		m_data = other.m_data;
		m_size = other.m_size;
	}

} // namespace snowboy
