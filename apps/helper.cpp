#include <fstream>
#include <helper.h>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>

std::vector<short> read_sample_file(const std::string& filename, bool treat_wave) {

	struct wavHeader { //44 byte HEADER only
		char RIFF[4];
		int RIFFsize;
		char fmt[8];
		int fmtSize;
		short fmtTag;
		short nchan;
		int fs;
		int avgBps;
		short nBlockAlign;
		short bps;
		char data[4];
		int datasize;
	};

	auto readWavHeader = [](wavHeader* wavhdr, FILE* fi) {
		char* tag = (char*)wavhdr;
		if (fread(wavhdr, 1, 34, fi) != 34) throw std::runtime_error("fread failed");
		if (tag[0] != 'R' || tag[1] != 'I' || tag[2] != 'F' || tag[3] != 'F')
		{
			throw std::runtime_error("not a riff file");
		}
		if (wavhdr->fmtTag != 1)
		{
			throw std::runtime_error("WAV file has encoded data or it is WAVEFORMATEXTENSIBLE");
		}
		if (wavhdr->fmtSize == 14)
		{
			wavhdr->bps = 16;
		}
		if (wavhdr->fmtSize >= 16)
		{
			if (fread(&wavhdr->bps, 1, 2, fi) != 2) throw std::runtime_error("fread failed");
		}
		if (wavhdr->fmtSize == 18)
		{
			short lixo;
			if (fread(&lixo, 1, 2, fi) != 2) throw std::runtime_error("fread failed");
		}
		tag += 36;
		if (fread(tag, 1, 4, fi) != 4) throw std::runtime_error("fread failed");
		while (tag[0] != 'd' || tag[1] != 'a' || tag[2] != 't' || tag[3] != 'a')
		{
			if (fread(tag, 1, 4, fi) != 4) throw std::runtime_error("fread failed");
			if (ftell(fi) >= long(wavhdr->RIFFsize))
			{
				fclose(fi);
				perror("Bad WAV header !");
			}
		}
		if (fread(&wavhdr->datasize, 1, 4, fi) != 4) throw std::runtime_error("fread failed");
	};

	bool isWav = treat_wave || (filename.size() > 4 && filename.rfind(".wav") == filename.size() - 4);

	FILE* f = fopen(filename.c_str(), "rb");
	if (f == NULL)
	{
		throw std::runtime_error("Error opening file");
		return {};
	}

	std::vector<short> res;
	try {
		if (isWav)
		{
			wavHeader wavhdr{};
			readWavHeader(&wavhdr, f);
			res.resize(wavhdr.datasize / 2);
		} else
		{
			fseek(f, 0, SEEK_END);
			res.resize(ftell(f) / 2);
			rewind(f);
		}
		auto read = fread(res.data(), 1, res.size() * 2, f);
		while (read != res.size() * 2) {
			auto r = fread(res.data() + read, res.size() * 2 - read, 1, f);
			if (r <= 0) throw std::runtime_error("fread failed: wanted=" + std::to_string(res.size() * 2) + " got=" + std::to_string(read));
			read += r;
		}
	} catch (...) {
		fclose(f);
		throw;
	}
	fclose(f);
	return res;
}

bool file_exists(const std::string& name) {
	struct stat buffer;
	return stat(name.c_str(), &buffer) == 0;
}

std::string detect_project_root() {
	std::string prefix;
	for (int i = 0; i < 5; i++) {
		if (file_exists(prefix + "./.git")) return prefix;
		prefix += "../";
	}
	return "";
}

std::string read_file(const std::string& file) {
	std::ifstream f{file, std::ios::binary};
	std::stringstream content;
	content << f.rdbuf();
	return content.str();
}

void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
				return !std::isspace(ch);
			}));
}

void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
				return !std::isspace(ch);
			}).base(),
			s.end());
}

void trim(std::string& s) {
	ltrim(s);
	rtrim(s);
}

std::vector<std::string> split(const std::string& s, const std::string& delim, size_t max) {
	std::vector<std::string> res;
	size_t offset = 0;
	do {
		auto pos = s.find(delim, offset);
		if (res.size() < max - 1 && pos != std::string::npos)
			res.push_back(s.substr(offset, pos - offset));
		else {
			res.push_back(s.substr(offset));
			break;
		}
		offset = pos + delim.size();
	} while (true);
	return res;
}

arg_iterator::arg_iterator(std::vector<std::string> a)
	: args(a) {
	current_token = args.begin();
}

arg_iterator::arg_iterator(int argc, const char** argv) {
	for (int i = 0; i < argc; i++)
		args.push_back(argv[i]);
	current_token = args.begin();
}

arg_iterator::arg_iterator(const std::string& str) {
	auto v = split(str, std::string(" "));
	for (auto& e : v) {
		trim(e);
		args.push_back(e);
	}
	current_token = args.begin();
}

bool arg_iterator::has_more() const noexcept {
	return current_token != args.end();
}

const std::string& arg_iterator::peek() const {
	if (!has_more()) throw std::logic_error("out of arguments");
	return *current_token;
}

const std::string& arg_iterator::take() {
	if (!has_more()) throw std::logic_error("out of arguments");
	return *current_token++;
}

option_base& option_base::set_shortname(std::string name) {
	shortname = std::move(name);
	return *this;
}

option_base& option_base::set_longname(std::string name) {
	longname = std::move(name);
	return *this;
}

option_base& option_base::set_description(std::string name) {
	description = std::move(name);
	return *this;
}

option_base& option_base::set_required(bool r) {
	required = r;
	return *this;
}

void bool_option::parse(arg_iterator& it) {
	*value_ptr = true;
	if (it.has_more() && it.peek() == "false") {
		*value_ptr = false;
		it.take();
	}
}

void string_option::parse(arg_iterator& it) {
	if (!it.has_more()) {
		throw std::runtime_error("missing argument for option " + longname);
	}
	*value_ptr = it.take();
}

void string_list_option::parse(arg_iterator& it) {
	if (!it.has_more()) {
		throw std::runtime_error("missing argument for option " + longname);
	}
	value_ptr->push_back(it.take());
}

void int_option::parse(arg_iterator& it) {
	if (!it.has_more()) {
		throw std::runtime_error("missing argument for option " + longname);
	}
	*value_ptr = std::stoll(it.take());
	if(*value_ptr < minimum || *value_ptr > maximum) {
		throw std::runtime_error("value exceeds range");
	}
}

int_option& int_option::set_min(int64_t m) noexcept {
	this->minimum = m;
	return *this;
}

int_option& int_option::set_max(int64_t m) noexcept {
	this->maximum = m;
	return *this;
}

option_parser::~option_parser() {
	for (auto& e : options)
		delete e;
	options.clear();
}

bool_option& option_parser::option(std::string longname, bool* ptr) {
	auto opt = new bool_option(ptr);
	opt->longname = longname;
	options.push_back(opt);
	return *opt;
}

string_option& option_parser::option(std::string longname, std::string* ptr) {
	auto opt = new string_option(ptr);
	opt->longname = longname;
	options.push_back(opt);
	return *opt;
}

string_list_option& option_parser::option(std::string longname, std::vector<std::string>* ptr) {
	auto opt = new string_list_option(ptr);
	opt->longname = longname;
	options.push_back(opt);
	return *opt;
}

int_option& option_parser::option(std::string longname, int64_t* ptr) {
	auto opt = new int_option(ptr);
	opt->longname = longname;
	options.push_back(opt);
	return *opt;
}

std::vector<std::string> option_parser::parse(std::vector<std::string> a) {
	arg_iterator it{a};
	return parse(it);
}

std::vector<std::string> option_parser::parse(int argc, const char** argv) {
	arg_iterator it{argc, argv};
	return parse(it);
}

std::vector<std::string> option_parser::parse(const std::string& str) {
	arg_iterator it{str};
	return parse(it);
}

std::vector<std::string> option_parser::parse(arg_iterator& it) {
	std::vector<std::string> extra_args;
	while (it.has_more()) {
		auto opt = it.take();
		option_base* o = nullptr;
		for (auto e : options) {
			if (opt == e->shortname || opt == e->longname) {
				o = e;
				break;
			}
		}
		if (o == nullptr) {
			extra_args.push_back(opt);
			continue;
		}
		o->parse(it);
	}
	return extra_args;
}

void option_parser::print_help(std::ostream& out) {
	size_t max_name = 0;
	size_t max_name_short = 0;
	for (auto e : options) {
		max_name_short = std::max(max_name_short, e->shortname.size());
		max_name = std::max(max_name, e->longname.size());
	}
	max_name += 2;
	max_name_short += 2;
	for (auto e : options) {
		out << e->shortname << std::string(max_name_short - (e->shortname.size()), ' ');
		out << e->longname << std::string(max_name - (e->longname.size()), ' ');
		out << e->description << "\n";
	}
}

std::string option_parser::help() {
	std::stringstream ss;
	print_help(ss);
	return ss.str();
}
