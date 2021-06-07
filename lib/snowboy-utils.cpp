#include <snowboy-error.h>
#include <snowboy-utils.h>
#include <string>

namespace snowboy {
	const std::string global_snowboy_whitespace_set{" \t\n\r\f\v"};

	std::string Basename(const std::string& file) {
		auto pos = file.find_last_of('/');
		if (pos == std::string::npos) return file;
		return file.substr(pos);
	}

	std::string CharToString(const char& c) {
		std::string res;
		res.resize(16);
		const char* fmt = "\'%c\'";
		if (!isprint(c)) fmt = "[character %d]";
		auto len = snprintf(const_cast<char*>(res.data()), res.size(), fmt, c);
		if (len < 0) return "";
		res.resize(len);
		return res;
	}

	bool ConvertStringToBoolean(const std::string& val) {
		return ConvertStringTo<bool>(val);
	}

	template <>
	bool ConvertStringTo<bool>(const std::string& val) {
		auto v = val;
		Trim(&v);
		if (v == "true") return true;
		if (v == "false") return false;
		throw snowboy_exception{"ConvertStringTo<bool>: Bad value for boolean type: " + val};
	}

	template <>
	float ConvertStringTo<float>(const std::string& val) {
		auto v = val;
		Trim(&v);
		try {
			return std::stof(v, nullptr);
		} catch (const std::exception& e) {
			throw snowboy_exception{"ConvertStringTo<float>: Bad value for boolean type: " + val};
		}
	}

	template <>
	int32_t ConvertStringTo<int32_t>(const std::string& val) {
		auto v = val;
		Trim(&v);
		try {
			return std::stoi(v, nullptr);
		} catch (const std::exception& e) {
			throw snowboy_exception{"ConvertStringTo<int32_t>: Bad value for boolean type: " + val};
		}
	}

	template <>
	uint32_t ConvertStringTo<uint32_t>(const std::string& val) {
		auto v = val;
		Trim(&v);
		try {
			return std::stoul(v, nullptr);
		} catch (const std::exception& e) {
			throw snowboy_exception{"ConvertStringTo<uint32_t>: Bad value for boolean type: " + val};
		}
	}

	void FilterConfigString(bool invert, const std::string& prefix, std::string* config_str) {
		if (prefix != "") {
			std::vector<std::string> parts;
			SplitStringToVector(*config_str, global_snowboy_whitespace_set, &parts);
			config_str->clear();
			for (auto& e : parts) {
				auto pos = e.find(prefix, 0);
				if ((pos != std::string::npos && !invert) || (pos == std::string::npos && invert)) {
					config_str->append(" ");
					config_str->append(e);
				}
			}
		}
	}

	void* SnowboyMemalign(size_t align, size_t size) {
		void* ptr = nullptr;
		if (posix_memalign(&ptr, align, size) == 0)
			return ptr;
		else
			return nullptr;
	}

	void SnowboyMemalignFree(void* ptr) {
		free(ptr);
	}

	template <>
	void SplitStringToIntegers(const std::string& s1, const std::string& s2, std::vector<int>* out) {
		std::vector<std::string> parts;
		SplitStringToVector(s1, s2, &parts);
		out->resize(parts.size());
		for (size_t i = 0; i < parts.size(); i++) {
			out->at(i) = ConvertStringToIntegerOrFloat<int>(parts[i]);
		}
	}

	template <>
	void SplitStringToIntegers(const std::string& s1, const char* s2, std::vector<int>* out) {
		SplitStringToIntegers(s1, std::string{s2}, out);
	}

	void SplitStringToFloats(const std::string& s1, const std::string& s2, std::vector<float>* out) {
		std::vector<std::string> parts;
		SplitStringToVector(s1, s2, &parts);
		out->resize(parts.size());
		for (size_t i = 0; i < parts.size(); i++) {
			out->at(i) = ConvertStringTo<float>(parts[i]);
		}
	}

	void SplitStringToFloats(const std::string& s1, const char* s2, std::vector<float>* out) {
		std::string stemp{s2};
		SplitStringToFloats(s1, stemp, out);
	}

	void SplitStringToVector(const std::string& s1, const std::string& s2, std::vector<std::string>* out) {
		size_t pos = 0;
		auto offset = pos;
		do {
			pos = s1.find_first_of(s2, offset);
			auto str = s1.substr(offset, pos - offset);
			if (!str.empty())
				out->push_back(std::move(str));
			offset = pos + 1;
		} while (pos != std::string::npos);
	}

	void SplitStringToVector(const std::string& s1, const char* s2, std::vector<std::string>* out) {
		std::string stemp{s2};
		return SplitStringToVector(s1, stemp, out);
	}

	void Trim(std::string* str) {
		TrimLeft(str);
		TrimRight(str);
	}

	void TrimLeft(std::string* str) {
		auto pos = str->find_first_not_of(global_snowboy_whitespace_set);
		str->erase(str->begin(), str->begin() + pos);
	}

	void TrimRight(std::string* str) {
		auto pos = str->find_last_not_of(global_snowboy_whitespace_set);
		if (pos != std::string::npos)
			str->resize(pos + 1);
	}
} // namespace snowboy
