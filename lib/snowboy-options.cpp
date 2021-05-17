#include <snowboy-debug.h>
#include <snowboy-options.h>
#include <snowboy-utils.h>
#include <sstream>

namespace snowboy {

	OptionInfo::OptionInfo(bool* ptr) {
		m_bool_value = ptr;
		m_type = kBool;
		m_default_value = *ptr ? "true" : "false";
		m_info = {};
	}

	OptionInfo::OptionInfo(std::string* ptr) {
		m_string_value = ptr;
		m_type = kString;
		m_default_value = *ptr;
		m_info = {};
	}

	OptionInfo::OptionInfo(uint32_t* ptr) {
		m_uint_value = ptr;
		m_type = kUint32;
		m_default_value = std::to_string(*ptr);
		m_info = {};
	}

	OptionInfo::OptionInfo(int32_t* ptr) {
		m_int_value = ptr;
		m_type = kInt32;
		m_default_value = std::to_string(*ptr);
		m_info = {};
	}

	OptionInfo::OptionInfo(float* ptr) {
		m_float_value = ptr;
		m_type = kFloat;
		m_default_value = std::to_string(*ptr);
		m_info = {};
	}

	std::string OptionInfo::GetActualMessage() const {
		std::stringstream ss;
		ss << m_info;
		switch (m_type) {
		case kBool: ss << " (bool, current = " << (*m_bool_value); break;
		case kInt32: ss << " (int32, current = " << (*m_int_value); break;
		case kUint32: ss << " (uint32, current = " << (*m_uint_value); break;
		case kFloat: ss << " (float, current = " << (*m_float_value); break;
		case kString: ss << " (string, current = " << (*m_string_value); break;
		default:
			SNOWBOY_ERROR() << "PointerType is not defined";
			return "";
		}
		ss << ")";
		return ss.str();
	}

	std::string OptionInfo::GetDefaultMessage() const {
		std::stringstream ss;
		ss << m_info;
		switch (m_type) {
		case kBool: ss << " (bool, default = "; break;
		case kInt32: ss << " (int32, default = "; break;
		case kUint32: ss << " (uint32, default = "; break;
		case kFloat: ss << " (float, default = "; break;
		case kString: ss << " (string, default = "; break;
		default:
			SNOWBOY_ERROR() << "PointerType is not defined";
			return "";
		}
		ss << m_default_value << ")";
		return ss.str();
	}

	void OptionInfo::SetValue(const std::string& v) {
		switch (m_type) {
		case kBool:
			if (v.empty())
				*m_bool_value = true;
			else
				*m_bool_value = ConvertStringTo<bool>(v);
			break;
		case kInt32: *m_int_value = ConvertStringTo<int32_t>(v); break;
		case kUint32: *m_uint_value = ConvertStringTo<uint32_t>(v); break;
		case kFloat: *m_float_value = ConvertStringTo<float>(v); break;
		case kString: *m_string_value = v; break;
		default:
			SNOWBOY_ERROR() << "PointerType is not defined";
			break;
		}
	}

	ParseOptions::ParseOptions(const std::string& usage) {
		m_opt_print_usage = false;
		m_usage = usage;
		Register("", "config", "Configuration file to be read.", &m_opt_config_file);
		Register("", "help", "If true, print usage information.", &m_opt_print_usage);
		Register("", "verbose", "Verbose level.", &global_snowboy_verbose_level);
	}

	ParseOptions::~ParseOptions() {}

	void ParseOptions::Register(const std::string& prefix, const std::string& name, const std::string& usage_info, bool* ptr) {
		auto full_name = prefix;
		if (!full_name.empty()) full_name += ".";
		full_name += name;
		full_name = NormalizeOptionName(full_name);
		auto it = m_options.emplace(full_name, OptionInfo{ptr});
		if (!it.second) {
			SNOWBOY_ERROR() << "Option --" << full_name << " has already been registered, try to use a prefix if you have option conflicts?";
			return;
		}
		it.first->second.m_info = usage_info;
	}

	void ParseOptions::Register(const std::string& prefix, const std::string& name, const std::string& usage_info, int32_t* ptr) {
		auto full_name = prefix;
		if (!full_name.empty()) full_name += ".";
		full_name += name;
		full_name = NormalizeOptionName(full_name);
		auto it = m_options.emplace(full_name, OptionInfo{ptr});
		if (!it.second) {
			SNOWBOY_ERROR() << "Option --" << full_name << " has already been registered, try to use a prefix if you have option conflicts?";
			return;
		}
		it.first->second.m_info = usage_info;
	}

	void ParseOptions::Register(const std::string& prefix, const std::string& name, const std::string& usage_info, uint32_t* ptr) {
		auto full_name = prefix;
		if (!full_name.empty()) full_name += ".";
		full_name += name;
		full_name = NormalizeOptionName(full_name);
		auto it = m_options.emplace(full_name, OptionInfo{ptr});
		if (!it.second) {
			SNOWBOY_ERROR() << "Option --" << full_name << " has already been registered, try to use a prefix if you have option conflicts?";
			return;
		}
		it.first->second.m_info = usage_info;
	}

	void ParseOptions::Register(const std::string& prefix, const std::string& name, const std::string& usage_info, float* ptr) {
		auto full_name = prefix;
		if (!full_name.empty()) full_name += ".";
		full_name += name;
		full_name = NormalizeOptionName(full_name);
		auto it = m_options.emplace(full_name, OptionInfo{ptr});
		if (!it.second) {
			SNOWBOY_ERROR() << "Option --" << full_name << " has already been registered, try to use a prefix if you have option conflicts?";
			return;
		}
		it.first->second.m_info = usage_info;
	}

	void ParseOptions::Register(const std::string& prefix, const std::string& name, const std::string& usage_info, std::string* ptr) {
		auto full_name = prefix;
		if (!full_name.empty()) full_name += ".";
		full_name += name;
		full_name = NormalizeOptionName(full_name);
		auto it = m_options.emplace(full_name, OptionInfo{ptr});
		if (!it.second) {
			SNOWBOY_ERROR() << "Option --" << full_name << " has already been registered, try to use a prefix if you have option conflicts?";
			return;
		}
		it.first->second.m_info = usage_info;
	}

	void ParseOptions::Remove(const std::string& prefix, const std::string& name) {
		auto full_name = prefix;
		if (!full_name.empty()) full_name += ".";
		full_name += name;
		full_name = NormalizeOptionName(full_name);
		auto it = m_options.find(full_name);
		if (it == m_options.end()) {
			SNOWBOY_ERROR() << "Option --" << full_name << " has not been registered.";
			return;
		}
		m_options.erase(it);
	}

	std::string ParseOptions::GetArgument(int index) const {
		// NOTE: Not in original and should never happen, but better be save than sorry...
		if (index + 1 > m_arguments.size() || index < 0) return "";
		return m_arguments[index];
	}

	bool ParseOptions::IsValidOption(const std::string& opt) const {
		if (opt.compare(0, 2, "--") != 0) return false;
		auto pos = opt.find_first_of('=', 2);
		if (pos == std::string::npos) return true;
		if (pos == opt.size() - 1) return false;
		return true;
	}

	std::string ParseOptions::NormalizeOptionName(const std::string& option) const {
		auto res = option;
		for (auto& e : res)
			e = tolower(e);
		return res;
	}

	void ParseOptions::ParseOneOption(const std::string& opt, std::string* out_name, std::string* out_value) const {
		auto v = opt;
		if (v.compare(0, 2, "--") == 0) v.erase(0, 2);
		auto pos = v.find_first_of('=');
		if (pos == std::string::npos) {
			*out_name = std::move(v);
			out_value->clear();
		} else {
			out_value->assign(v.begin() + pos + 1, v.end());
			v.resize(pos);
			*out_name = std::move(v);
		}
	}

	void ParseOptions::PrintUsage(bool) {
		// NOTE: Nothing here in original....
		return;
	}

	void ParseOptions::ReadArguments(int argc, char const* const* argv) {
		std::string opts;
		for (int i = 0; i < argc; i++) {
			// Note: this was in the original, but is not needed since ReadConfigString checks as well
			//if(!IsValidOption(argv[i])) {
			//    SNOWBOY_ERROR() << "Invalid option: " << argv[i] << "; supported format is --option=value or --option for boolean types.";
			//    return;
			//}
			if (!opts.empty()) opts += " ";
			opts += argv[i];
		}
		// TODO: This is how its done in the original, but the other way arount is probably more performant.
		ReadConfigString(opts);
	}

	void ParseOptions::ReadConfigFile(const std::string& filename) {
		// TODO: Implement...
		SNOWBOY_ERROR() << "Unimplemented!";
	}

	void ParseOptions::ReadConfigString(const std::string& config) {
		std::vector<std::string> parts;
		SplitStringToVector(config, " ", &parts);
		std::string name;
		std::string value;
		for (auto& e : parts) {
			if (!IsValidOption(e)) {
				SNOWBOY_ERROR() << "Invalid option: " << e << "; supported format is --option=value or --option for boolean types.";
				return;
			}
			ParseOneOption(e, &name, &value);
			if (name == "config") {
				ReadConfigFile(value);
				break;
			}
		}
		for (auto& e : parts) {
			if (!IsValidOption(e)) {
				SNOWBOY_ERROR() << "Invalid option: " << e << "; supported format is --option=value or --option for boolean types.";
				return;
			}
			ParseOneOption(e, &name, &value);
			if (name == "config") continue;
			if (name == "help") continue;
			auto it = m_options.find(name);
			if (it == m_options.end()) {
				SNOWBOY_ERROR() << "Undefined option: " << name;
				return;
			}
			it->second.SetValue(value);
		}
	}

} // namespace snowboy