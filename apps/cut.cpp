#include <cstring>
#include <framer-stream.h>
#include <fstream>
#include <helper.h>
#include <iostream>
#include <pipeline-personal-enroll.h>
#include <snowboy-detect.h>
#include <template-container.h>
#include <template-enroll-stream.h>
#include <types.h>

const static auto root = detect_project_root();

bool parse_args(int argc, const char** argv, std::string& output, std::string& recording, std::string& lang);

int main(int argc, const char** argv) {
	std::string output, recording, lang;
	if (!parse_args(argc, argv, output, recording, lang)) return -1;

	snowboy::SnowboyTemplateCut cut{root + "resources/pmdl/" + lang + "/personal_enroll.res"};
	auto data = read_sample_file(recording, true);
	std::string str_data;
	str_data.resize(data.size() * 2);
	memcpy(const_cast<char*>(str_data.data()), data.data(), str_data.size());

	auto res = cut.CutTemplate(str_data);

	std::ofstream file{output, std::ios::binary};
	file.write(reinterpret_cast<const char*>(cut.wave_header_.get()), sizeof(snowboy::WaveHeader));
	file.write(res.data(), res.size());
	return 0;
}

bool parse_args(int argc, const char** argv, std::string& output, std::string& recording, std::string& lang) {
	std::vector<std::string> extra_args;
	for (int i = 1; i < argc; i++) {
		if (argv[i] == std::string("-r")) {
			if (i == argc - 1) {
				std::cerr << "Missing parameter for arg -r" << std::endl;
				return false;
			}
			recording = argv[i + 1];
			i++;
		} else if (argv[i] == std::string("-lang")) {
			if (i == argc - 1) {
				std::cerr << "Missing parameter for arg -lang" << std::endl;
				return false;
			}
			lang = argv[i + 1];
			i++;
		} else if (argv[i] == std::string("-o")) {
			if (i == argc - 1) {
				std::cerr << "Missing parameter for arg -o" << std::endl;
				return false;
			}
			output = argv[i + 1];
			i++;
		} else {
			extra_args.push_back(argv[i]);
		}
	}
	if (recording.empty() && !extra_args.empty()) {
		recording = extra_args.front();
		extra_args.erase(extra_args.begin());
	}
	if (output.empty() && !extra_args.empty()) {
		output = extra_args.front();
		extra_args.erase(extra_args.begin());
	}
	if (lang.empty() && !extra_args.empty()) {
		lang = extra_args.front();
		extra_args.erase(extra_args.begin());
	}

	if (output.empty() && !recording.empty()) {
		auto pos = recording.rfind(".wav");
		if (pos == recording.size() - 4) {
			output = recording.substr(0, recording.size() - 4) + "_cut.wav";
		} else
			output = recording + "_cut.wav";
	}
	if (lang.empty()) lang = "en";
	if (output.empty() || recording.empty() || lang.empty()) {
		std::cerr << "Missing required argument" << std::endl;
		return false;
	}
	if (lang != "en" && lang != "zh") {
		std::cerr << "Invalid value \"" << lang << "\" for language parameter. Allowed values: \"en\", \"zh\"." << std::endl;
		return false;
	}
	return true;
}