#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace snowboy {
	struct OptionInfo {
		enum type {
			kBool = 0x2,
			kInt32 = 0x4,
			kUint32 = 0x8,
			kFloat = 0x10,
			kString = 0x20
		};

		std::string m_default_value;
		std::string m_info;
		union
		{
			bool* m_bool_value;
			int32_t* m_int_value;
			uint32_t* m_uint_value;
			float* m_float_value;
			std::string* m_string_value;
		};
		type m_type;

		OptionInfo(bool* ptr);
		OptionInfo(std::string* ptr);
		OptionInfo(uint32_t* ptr);
		OptionInfo(int32_t* ptr);
		OptionInfo(float* ptr);

		std::string GetActualMessage() const;
		std::string GetDefaultMessage() const;

		void SetValue(const std::string& v);
	};

	struct OptionsItf {
		virtual void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, bool* ptr) = 0;
		virtual void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, int32_t* ptr) = 0;
		virtual void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, uint32_t* ptr) = 0;
		virtual void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, float* ptr) = 0;
		virtual void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, std::string* ptr) = 0;
		virtual void Remove(const std::string& prefix, const std::string& name) = 0;
		virtual ~OptionsItf() {}
	};

	class ParseOptions : public OptionsItf {
		bool m_opt_print_usage;
		std::string m_opt_config_file;
		std::string m_usage;
		std::vector<std::string> m_arguments;
		std::unordered_map<std::string, OptionInfo> m_options;
		int m_verbose_level;

	public:
		ParseOptions(const std::string& usage);
		~ParseOptions();

		void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, bool* ptr) override;
		void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, int32_t* ptr) override;
		void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, uint32_t* ptr) override;
		void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, float* ptr) override;
		void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, std::string* ptr) override;
		void Remove(const std::string& prefix, const std::string& name) override;

		std::string GetArgument(int index) const;
		bool IsValidOption(const std::string& opt) const;
		std::string NormalizeOptionName(const std::string& option) const;
		void ParseOneOption(const std::string& opt, std::string* out_name, std::string* out_value) const;
		void PrintUsage(bool);
		void ReadArguments(int argc, char const* const* argv);
		void ReadConfigFile(const std::string& filename);
		void ReadConfigString(const std::string& config);
	};
} // namespace snowboy
