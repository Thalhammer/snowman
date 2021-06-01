#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace snowboy {
	struct OptionsItf {
		virtual void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, bool* ptr) = 0;
		virtual void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, int32_t* ptr) = 0;
		virtual void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, uint32_t* ptr) = 0;
		virtual void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, int64_t* ptr) = 0;
		virtual void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, uint64_t* ptr) = 0;
		virtual void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, float* ptr) = 0;
		virtual void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, std::string* ptr) = 0;
		virtual void Remove(const std::string& prefix, const std::string& name) = 0;
		virtual ~OptionsItf() {}
	};

	struct OptionInfo;
	class ParseOptions : public OptionsItf {
		bool m_opt_print_usage;
		std::string m_opt_config_file;
		std::string m_usage;
		std::vector<std::string> m_arguments;
		std::map<std::string, OptionInfo> m_options;
		int m_verbose_level;

		void Register(const std::string& prefix, const std::string& name, const std::string usage_info, OptionInfo&& info);

	public:
		ParseOptions(const std::string& usage);
		~ParseOptions();

		void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, bool* ptr) override;
		void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, int32_t* ptr) override;
		void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, uint32_t* ptr) override;
		void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, int64_t* ptr) override;
		void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, uint64_t* ptr) override;
		void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, float* ptr) override;
		void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, std::string* ptr) override;
		void Remove(const std::string& prefix, const std::string& name) override;

		std::string GetArgument(size_t index) const;
		bool IsValidOption(const std::string& opt) const;
		std::string NormalizeOptionName(const std::string& option) const;
		void ParseOneOption(const std::string& opt, std::string* out_name, std::string* out_value) const;
		void PrintUsage(bool);
		void ReadArguments(int argc, char const* const* argv);
		void ReadConfigFile(const std::string& filename);
		void ReadConfigString(const std::string& config);
	};
} // namespace snowboy
