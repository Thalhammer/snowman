#pragma once
#include <gtest/gtest.h>
#include <matrix-wrapper.h>
#include <ostream>
#include <string>
#include <vector>

template <typename... Args>
void GTEST_WARN(const std::string& fmt, Args... args) {
	std::string msg = "[          ] " + fmt + "\n";
	testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, msg.c_str(), std::forward<Args>(args)...);
}

std::vector<short> read_sample_file(const std::string& filename, bool treat_wave = false);
std::string read_sample_file_as_string(const std::string& filename, bool treat_wave = false);
bool file_exists(const std::string& name);
std::string detect_project_root();
std::string read_file(const std::string& file);
std::string md5sum(const std::string& data);
std::string md5sum(const void* const data, size_t len);
std::string md5sum_file(const std::string& file);

inline size_t hash(const snowboy::MatrixBase& b) {
	std::hash<int> h;
	size_t res = 0;
	for (int r = 0; r < b.m_rows; r++) {
		for (int c = 0; c < b.m_cols; c++) {
			res += h(b.m_data[r * b.m_stride + c] * 10000);
		}
	}
	return res;
}

inline snowboy::Matrix random_matrix(unsigned int* seed) {
	snowboy::Matrix m;
	m.Resize(10 + rand_r(seed) % 50, 10 + rand_r(seed) % 50);
	for (size_t i = 0; i < m.m_rows * m.m_stride; i++) {
		m.m_data[i] = (rand_r(seed) % 10000) / 1000;
	}
	return m;
}

template <typename T>
std::ostream& operator<<(std::ostream& s, const std::vector<T>& o);

template <typename T>
std::ostream& operator<<(std::ostream& s, const std::vector<T>& o) {
	s << "vector<" << typeid(T).name() << "> [" << o.size() << "] {";
	for (auto& e : o)
		s << " " << e;
	s << " }";
	return s;
}
#define MEMCHECK_ENABLED (!defined(__SANITIZE_ADDRESS__) || __SANITIZE_ADDRESS__ != 1)
struct MemoryChecker {
	struct stacktrace {
		void* trace[50];
		void capture();
	};

	struct snapshot {
		ssize_t num_malloc = 0;
		ssize_t num_malloc_failed = 0;
		ssize_t num_malloc_bytes = 0;
		ssize_t num_free = 0;
		ssize_t num_realloc = 0;
		ssize_t num_realloc_failed = 0;
		ssize_t num_realloc_moved = 0;
		ssize_t num_realloc_bytes = 0;
		ssize_t num_memalign = 0;
		ssize_t num_memalign_bytes = 0;
		ssize_t num_memalign_failed = 0;
		ssize_t num_chunks_allocated = 0;
		ssize_t num_chunks_allocated_max = 0;
		ssize_t num_bytes_allocated = 0;
		ssize_t num_bytes_allocated_max = 0;
	};
	snapshot info;

	MemoryChecker();
	~MemoryChecker();
};

std::ostream& operator<<(std::ostream& str, const MemoryChecker::stacktrace& o);
std::ostream& operator<<(std::ostream& str, const MemoryChecker& o);

#if MEMCHECK_ENABLED
#define MEMCHECK_START() MemoryChecker check{};
#define MEMCHECK_REPORT() check
#define MEMCHECK_DUMP() std::cout << MEMCHECK_REPORT() << std::endl;
#define MEMCHECK_ASSERT_MAXMEM_LE(x) ASSERT_LE(check.info.num_bytes_allocated_max, x)
#else
#define MEMCHECK_START() do {} while (false)
#define MEMCHECK_REPORT() "<memcheck disabled due to asan>"
#define MEMCHECK_DUMP() do {} while (false)
#define MEMCHECK_ASSERT_MAXMEM_LE(x) ASSERT_TRUE(true)
#endif
