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
		// TODO: This might be an artifact of std::map, since its unused in OptionInfo
		char data[28];

		OptionInfo(bool* ptr);
		OptionInfo(std::string* ptr);
		OptionInfo(uint32_t* ptr);
		OptionInfo(int32_t* ptr);
		OptionInfo(float* ptr);

		std::string GetActualMessage() const;
		std::string GetDefaultMessage() const;

		void SetValue(const std::string& v);
	};
	static_assert(sizeof(OptionInfo) == 0x38);
	struct OptionsItf {
		virtual void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, bool* ptr) = 0;
		virtual void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, int32_t* ptr) = 0;
		virtual void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, uint32_t* ptr) = 0;
		virtual void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, float* ptr) = 0;
		virtual void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, std::string* ptr) = 0;
		virtual void Remove(const std::string&, const std::string&) = 0;
		virtual ~OptionsItf() {}
	};

	struct ParseOptions : OptionsItf {
		bool m_opt_print_usage;
		std::string m_opt_config_file;
		std::string m_usage;
		std::vector<std::string> m_arguments;
		std::unordered_map<std::string, OptionInfo> m_options;
		std::unordered_map<std::string, std::string> field_0x70;

		ParseOptions(const std::string& usage);
		~ParseOptions();

		void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, bool* ptr) override;
		void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, int32_t* ptr) override;
		void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, uint32_t* ptr) override;
		void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, float* ptr) override;
		void Register(const std::string& prefix, const std::string& name, const std::string& usage_info, std::string* ptr) override;
		void Remove(const std::string&, const std::string&) override;

		std::string GetArgument(int index) const;
		bool IsValidOption(const std::string& opt) const;
		std::string NormalizeOptionName(const std::string& option) const;
		// Remove leading -- and split on =
		void ParseOneOption(const std::string& opt, std::string* out_name, std::string* out_value) const;
		void PrintUsage(bool);
		void ReadArguments(int argc, char const* const* argv);
		void ReadConfigFile(const std::string& filename);
		void ReadConfigString(const std::string& config);
	};
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
	static_assert(offsetof(ParseOptions, m_opt_print_usage) == 0x8);
	static_assert(offsetof(ParseOptions, m_opt_config_file) == 0x10);
	static_assert(offsetof(ParseOptions, m_usage) == 0x18);
	static_assert(offsetof(ParseOptions, m_arguments) == 0x20);
	static_assert(offsetof(ParseOptions, m_options) == 0x38);
	static_assert(offsetof(ParseOptions, field_0x70) == 0x70);
#pragma GCC diagnostic pop
	// TODO: This might be wrong, since we dont have a new/malloc call for it.
	static_assert(sizeof(ParseOptions) == 0xa8);
} // namespace snowboy