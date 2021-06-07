#pragma once
#include <string>
#include <vector>

namespace snowboy {
	std::string Basename(const std::string& file);
	std::string CharToString(const char& c);
	template <typename T>
	T ConvertStringTo(const std::string&);
	template <>
	bool ConvertStringTo(const std::string&);
	template <>
	float ConvertStringTo(const std::string&);
	bool ConvertStringToBoolean(const std::string& val);
	template <typename T>
	inline T ConvertStringToIntegerOrFloat(const std::string& s) { return ConvertStringTo<T>(s); }
	void FilterConfigString(bool, const std::string& prefix, std::string* config_str);
	void* SnowboyMemalign(size_t align, size_t size);
	void SnowboyMemalignFree(void* ptr);
	template <typename T>
	void SplitStringToIntegers(const std::string& s1, const std::string& s2, std::vector<T>* out);
	template <typename T>
	void SplitStringToIntegers(const std::string& s1, const char* s2, std::vector<T>* out);
	template <>
	void SplitStringToIntegers(const std::string& s1, const std::string& s2, std::vector<int>* out);
	template <>
	void SplitStringToIntegers(const std::string& s1, const char* s2, std::vector<int>* out);
	void SplitStringToFloats(const std::string& s1, const std::string& s2, std::vector<float>* out);
	void SplitStringToFloats(const std::string& s1, const char* s2, std::vector<float>* out);
	void SplitStringToVector(const std::string& s1, const std::string& s2, std::vector<std::string>* out);
	void SplitStringToVector(const std::string& s1, const char* s2, std::vector<std::string>* out);
	void Trim(std::string* str);
	void TrimLeft(std::string* str);
	void TrimRight(std::string* str);
	extern const std::string global_snowboy_whitespace_set;
} // namespace snowboy
