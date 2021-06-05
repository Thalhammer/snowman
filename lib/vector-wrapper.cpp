extern "C"
{
#include <cblas.h>
}
#include <cmath>
#include <cstring>
#include <limits>
#include <matrix-wrapper.h>
#include <random>
#include <snowboy-error.h>
#include <snowboy-io.h>
#include <vector-wrapper.h>

namespace snowboy {
	void VectorBase::Add(float x) noexcept {
		for (size_t i = 0; i < m_size; i++) {
			m_data[i] += x;
		}
	}

	void VectorBase::AddDiagMat2(float param_1, const MatrixBase& param_2, MatrixTransposeType param_3, float param_4) noexcept {
		if (param_3 == MatrixTransposeType::kNoTrans) {
			auto ptr = param_2.m_data;
			for (size_t i = 0; i < m_size; i++) {
				auto fVar1 = m_data[i];
				auto fVar7 = cblas_sdot(param_2.m_cols, ptr, 1, ptr, 1);
				m_data[i] = fVar7 * param_1 + param_4 * fVar1;
				ptr += param_2.m_stride;
			}
		} else {
			for (size_t i = 0; i < m_size; i++) {
				auto fVar1 = m_data[i];
				auto fVar7 = cblas_sdot(param_2.m_rows, &param_2.m_data[i], param_2.m_stride, &param_2.m_data[i], param_2.m_stride);
				m_data[i] = fVar7 * param_1 + param_4 * fVar1;
			}
		}
	}

	void VectorBase::AddMatVec(float param_1, const MatrixBase& param_2, MatrixTransposeType param_3, const VectorBase& param_4, float param_5) noexcept {
		cblas_sgemv(CBLAS_ORDER::CblasRowMajor, static_cast<CBLAS_TRANSPOSE>(param_3),
					param_2.m_rows, param_2.m_cols, param_1, param_2.m_data, param_2.m_stride, param_4.m_data, 1, param_5, m_data, 1);
	}

	void VectorBase::AddVec(float param_1, const VectorBase& param_2) noexcept {
		SNOWBOY_ASSERT(param_2.m_size >= m_size);
		cblas_saxpy(m_size, param_1, param_2.m_data, 1, m_data, 1);
	}

	void VectorBase::AddVec2(float param_1, const VectorBase& param_2) noexcept {
		SNOWBOY_ASSERT(param_2.m_size >= m_size);
		for (size_t i = 0; i < m_size; i++)
			m_data[i] += param_1 * param_2.m_data[i] * param_2.m_data[i];
	}

	void VectorBase::ApplyFloor(float param_1) noexcept {
		for (size_t i = 0; i < m_size; i++) {
			m_data[i] = std::max(param_1, m_data[i]);
		}
	}

	void VectorBase::ApplyLog() noexcept {
		for (size_t i = 0; i < m_size; i++) {
			m_data[i] = logf(m_data[i]);
		}
	}

	void VectorBase::ApplyPow(float param_1) noexcept {
		for (size_t i = 0; i < m_size; i++) {
			m_data[i] = pow(m_data[i], param_1);
		}
	}

	float VectorBase::ApplySoftmax() noexcept {
		auto max = Max(), sum = 0.0f;
		for (size_t i = 0; i < m_size; i++) {
			m_data[i] = expf(m_data[i] - max);
			sum += m_data[i];
		}
		Scale(1.0f / sum);
		return logf(sum) + max;
	}

	void VectorBase::CopyColsFromMat(const MatrixBase& param_1) noexcept {
		SNOWBOY_ASSERT(m_size >= param_1.rows() * param_1.cols());
		for (size_t r = 0; r < param_1.rows(); r += 1) {
			for (size_t c = 0; c < param_1.cols(); c += 1) {
				m_data[r + param_1.m_rows * c] = param_1(r, c);
			}
		}
	}

	void VectorBase::CopyFromVec(const VectorBase& param_1) noexcept {
		if (m_data != param_1.m_data && m_data != nullptr && param_1.m_data != nullptr) {
			memcpy(m_data, param_1.m_data, std::min(m_size, param_1.m_size) * sizeof(float));
		}
	}

	void VectorBase::CopyRowsFromMat(const MatrixBase& param_1) noexcept {
		SNOWBOY_ASSERT(m_size >= param_1.cols() * param_1.rows());
		if (param_1.cols() != param_1.stride()) {
			for (size_t r = 0; r != param_1.rows(); r++) {
				memcpy(&m_data[r * param_1.cols()], &param_1.m_data[param_1.stride() * r], param_1.cols() * sizeof(float));
			}
		} else {
			memcpy(m_data, param_1.m_data, param_1.cols() * param_1.rows() * sizeof(float));
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

	bool VectorBase::IsZero(float cutoff) const noexcept {
		auto max = 0.0f;
		for (uint32_t i = 0; i < m_size; i++) {
			max = std::max(std::abs(m_data[i]), max);
		}
		return max <= cutoff;
	}

	float VectorBase::Max() const noexcept {
		auto max = -std::numeric_limits<float>::infinity();
		for (uint32_t i = 0; i < m_size; i++) {
			max = std::max(m_data[i], max);
		}
		return max;
	}

	float VectorBase::Max(int* e) const noexcept {
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

	float VectorBase::Min() const noexcept {
		auto min = std::numeric_limits<float>::infinity();
		for (uint32_t i = 0; i < m_size; i++) {
			min = std::min(m_data[i], min);
		}
		return min;
	}

	float VectorBase::Min(int* e) const noexcept {
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

	void VectorBase::MulElements(const VectorBase& param_1) noexcept {
		for (uint32_t i = 0; i < std::min(m_size, param_1.m_size); i++) {
			m_data[i] *= param_1.m_data[i];
		}
	}

	float VectorBase::Norm(float p) const noexcept {
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

	SubVector VectorBase::Range(size_t offset, size_t size) const noexcept {
		return SubVector{*this, offset, size};
	}

	void VectorBase::Scale(float factor) noexcept {
		cblas_sscal(m_size, factor, m_data, 1);
	}

	void VectorBase::Set(float val) noexcept {
		for (uint32_t i = 0; i < m_size; i++) {
			m_data[i] = val;
		}
	}

	void VectorBase::SetRandomGaussian() {
		throw snowboy_exception{"Not implemented"};
	}

	void VectorBase::SetRandomUniform() {
		throw snowboy_exception{"Not implemented"};
	}

	float VectorBase::Sum() const noexcept {
		auto sum = 0.0f;
		for (uint32_t i = 0; i < m_size; i++)
			sum += m_data[i];
		return sum;
	}

	void VectorBase::Write(bool binary, std::ostream* os) const {
		if (!*os) throw snowboy_exception{"Failed to write Vector to stream"};
		if (!binary) {
			*os << " [ ";
			for (uint32_t i = 0; i < m_size; i++) {
				*os << m_data[i] << " ";
			}
			*os << "]\n";
		} else {
			WriteToken(binary, "FV", os);
			WriteBasicType<int32_t>(binary, m_size, os);
			os->write(reinterpret_cast<const char*>(m_data), m_size * sizeof(float));
		}
		if (!*os) throw snowboy_exception{"Failed to write Vector to stream"};
	}

	bool VectorBase::HasNan() const noexcept {
		for (size_t i = 0; i < size(); i++) {
			if (m_data[i] != m_data[i]) return true;
		}
		return false;
	}

	bool VectorBase::HasInfinity() const noexcept {
		for (size_t i = 0; i < size(); i++) {
			if (std::isinf(m_data[i])) return true;
		}
		return false;
	}

	static size_t allocs = 0;
	static size_t frees = 0;
	void Vector::Resize(size_t size, MatrixResizeType resize) {
		SNOWBOY_ASSERT(m_size <= m_cap);
		if (size <= m_cap) {
#ifndef NDEBUG
			for (uint32_t i = m_size; i < size; i++) {
				m_data[i] = std::nanf("");
			}
#endif
			m_size = size;
			if (resize == MatrixResizeType::kSetZero) Set(0.0f);
			return;
		}

		allocs++;
		auto ptr = static_cast<float*>(SnowboyMemalign(16, size * sizeof(float)));
		if (ptr == nullptr) throw std::bad_alloc();
		if (resize == MatrixResizeType::kCopyData)
			memcpy(ptr, m_data, m_size * sizeof(float));
		if (m_data) {
			frees++;
			free(m_data);
		}
		if (resize == MatrixResizeType::kCopyData)
			memset(&ptr[m_size], 0, (size - m_size) * sizeof(float));
		else if (resize == MatrixResizeType::kSetZero)
			memset(ptr, 0, size * sizeof(float));
		m_data = ptr;
		m_size = size;
		m_cap = size;
	}

	Vector::~Vector() noexcept {
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
		Resize(other.size(), MatrixResizeType::kUndefined);
		CopyFromVec(other);
		return *this;
	}

	void Vector::Read(bool binary, bool add, std::istream* is) {
		if (!binary) {
			// TODO: Is this still accurate ?
			throw snowboy_exception{"Not implemented"};
			ExpectToken(binary, "[", is);
			uint32_t i = 0;
			auto s = m_size;
			for (; i < m_size; i++) {
				float f = 0.0f;
				if (!isspace(is->get())) {
					throw snowboy_exception{"Expecting space after number"};
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
					throw snowboy_exception{"Expecting space after numbers"};
				}
				if (is->get() != ']') {
					throw snowboy_exception{"Expecting closing bracket after data"};
				}
			}
			if (is->get() != '\n') {
				throw snowboy_exception{"Expecting newline after data"};
			}
		} else {
			ExpectToken(binary, "FV", is);
			int size;
			ReadBasicType<int32_t>(binary, &size, is);
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

	void Vector::Swap(Vector* other) noexcept {
		std::swap(m_data, other->m_data);
		std::swap(m_size, other->m_size);
		std::swap(m_cap, other->m_cap);
	}

	void Vector::RemoveElement(size_t index) noexcept {
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

	SubVector::SubVector(const VectorBase& parent, size_t offset, size_t size) noexcept {
		offset = std::min(offset, parent.size());
		m_data = parent.data() + offset;
		m_size = std::min(parent.size() - offset, size);
	}

	SubVector::SubVector(const MatrixBase& parent, size_t row) noexcept {
		m_data = parent.data(row);
		m_size = parent.m_cols;
	}

} // namespace snowboy
