#include <cstring>
#include <helper.h>
#include <iostream>
#include <snowboy-detect.h>

const static auto root = detect_project_root();

bool parse_args(int argc, const char** argv, std::string& output, std::vector<std::string>& recordings, std::string& lang, bool& cut_recordings);

int main(int argc, const char** argv) {
	std::string output, lang;
	std::vector<std::string> recordings;
	bool cut_recordings;
	if (!parse_args(argc, argv, output, recordings, lang, cut_recordings)) return -1;

	snowboy::SnowboyPersonalEnroll enroll{root + "resources/pmdl/" + lang + "/personal_enroll.res", output};
	snowboy::SnowboyTemplateCut cut{root + "resources/pmdl/" + lang + "/personal_enroll.res"};
	for (auto& e : recordings) {
		auto data = read_sample_file(e, true);
		std::string str_data;
		str_data.resize(data.size() * 2);
		memcpy(const_cast<char*>(str_data.data()), data.data(), str_data.size());

		if (cut_recordings) {
			str_data = cut.CutTemplate(str_data);
		}

		auto res = enroll.RunEnrollment(str_data);
		if (res != 0) {
			std::cerr << "Training failed with error " << res << std::endl;
			return -1;
		}
	}
	return 0;
}

bool parse_args(int argc, const char** argv, std::string& output, std::vector<std::string>& recordings, std::string& lang, bool& cut_recordings) {
	cut_recordings = true;
	std::vector<std::string> extra_args;
	for (int i = 1; i < argc; i++) {
		if (argv[i] == std::string("-n")) {
			if (i == argc - 1) {
				std::cerr << "Missing parameter for arg -n" << std::endl;
				return false;
			}
			output = argv[i + 1];
			i++;
		} else if (argv[i] == std::string("-r") || argv[i] == std::string("-r1") || argv[i] == std::string("-r2") || argv[i] == std::string("-r3")) {
			if (i == argc - 1) {
				std::cerr << "Missing parameter for arg -r" << std::endl;
				return false;
			}
			recordings.push_back(argv[i + 1]);
			i++;
		} else if (argv[i] == std::string("-lang")) {
			if (i == argc - 1) {
				std::cerr << "Missing parameter for arg -lang" << std::endl;
				return false;
			}
			lang = argv[i + 1];
			i++;
		} else if (argv[i] == std::string("-nc")) {
			cut_recordings = false;
		} else {
			extra_args.push_back(argv[i]);
		}
	}
	if (output.empty() && !extra_args.empty()) {
		output = extra_args.front();
		extra_args.erase(extra_args.begin());
	}
	if (recordings.empty() && !extra_args.empty()) {
		recordings = std::move(extra_args);
	}
	if (!recordings.empty() && (recordings.size() < 3 || (recordings.size() % 2) != 1)) {
		std::cerr << "Invalid number of recordings, you need to specify an odd number of recordings (" << recordings.size() << "), but at least 3 (e.g. 3, 5, 7, ...)." << std::endl;
		return false;
	}
	if (output.empty()) output = "model.pmdl";
	if (recordings.empty()) {
		recordings.push_back("record1.wav");
		recordings.push_back("record2.wav");
		recordings.push_back("record3.wav");
	}
	if (lang.empty()) lang = "en";
	return true;
}
