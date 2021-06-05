#pragma once
#include <algorithm>
#include <limits>
#include <string>
#include <vector>

std::vector<short> read_sample_file(const std::string& filename, bool treat_wave = false);
bool file_exists(const std::string& name);
std::string detect_project_root();
std::string read_file(const std::string& file);

void ltrim(std::string& s);
void rtrim(std::string& s);
void trim(std::string& s);
std::vector<std::string> split(const std::string& s, const std::string& delim, size_t max = std::numeric_limits<size_t>::max());

struct arg_iterator {
	std::vector<std::string> args;
	std::vector<std::string>::iterator current_token;

	arg_iterator(std::vector<std::string> a);
	arg_iterator(int argc, const char** argv);
	arg_iterator(const std::string& str);
	bool has_more() const noexcept;
	const std::string& peek() const;
	const std::string& take();
};

struct option_base {
	std::string shortname;
	std::string longname;
	std::string description;
	bool required;

	option_base& set_shortname(std::string name);
	option_base& set_longname(std::string name);
	option_base& set_description(std::string name);
	option_base& set_required(bool r);

	virtual ~option_base() {}
	virtual void parse(arg_iterator&) = 0;
};

template <typename T>
struct basic_option : option_base {
	T* value_ptr;

	basic_option(T* ptr) : value_ptr(ptr) {}
};

struct bool_option : basic_option<bool> {
	using basic_option::basic_option;
	void parse(arg_iterator& it);
};
struct string_option : basic_option<std::string> {
	using basic_option::basic_option;
	void parse(arg_iterator& it);
};
struct string_list_option : basic_option<std::vector<std::string>> {
	using basic_option::basic_option;
	void parse(arg_iterator& it);
};

struct int_option : basic_option<int64_t> {
	int64_t minimum = INT64_MIN;
	int64_t maximum = INT64_MAX;
	using basic_option::basic_option;
	void parse(arg_iterator& it);

	int_option& set_min(int64_t minimum) noexcept;
	int_option& set_max(int64_t maximum) noexcept;
};

struct option_parser {
	std::vector<option_base*> options;

	option_parser() {}
	option_parser(const option_parser&) = delete;
	option_parser& operator=(const option_parser&) = delete;
	option_parser(option_parser&) = delete;
	option_parser& operator=(option_parser&) = delete;
	~option_parser();
	bool_option& option(std::string longname, bool* ptr);
	string_option& option(std::string longname, std::string* ptr);
	string_list_option& option(std::string longname, std::vector<std::string>* ptr);
	int_option& option(std::string longname, int64_t* ptr);
	std::vector<std::string> parse(std::vector<std::string> a);
	std::vector<std::string> parse(int argc, const char** argv);
	std::vector<std::string> parse(const std::string& str);
	std::vector<std::string> parse(arg_iterator& it);
	void print_help(std::ostream&);
	std::string help();
};
