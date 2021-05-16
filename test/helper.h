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
std::string md5sum_file(const std::string& file);

inline size_t hash(const snowboy::MatrixBase& b) {
	std::hash<int> h;
	size_t res = 0;
	for (int r = 0; r < b.m_rows; r++) {
		for (int c = 0; c < b.m_cols; c++) {
			res += h(b.m_data[r * b.m_stride + c] * 1000);
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