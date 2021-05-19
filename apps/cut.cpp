#include <cstring>
#include <fstream>
#include <helper.h>
#include <iostream>
#include <snowboy-detect.h>
#include <wave-header.h>

const static auto root = detect_project_root();

bool parse_args(int argc, const char** argv, std::string& output, std::string& recording, std::string& lang);

int main(int argc, const char** argv) {
	std::string output, recording, lang;
	if (!parse_args(argc, argv, output, recording, lang)) return -1;
	if (output.empty()) return 0;

	snowboy::SnowboyTemplateCut cut{root + "resources/pmdl/" + lang + "/personal_enroll.res"};
	auto data = read_sample_file(recording, true);
	std::string str_data;
	str_data.resize(data.size() * 2);
	memcpy(const_cast<char*>(str_data.data()), data.data(), str_data.size());

	auto res = cut.CutTemplate(str_data);

	std::ofstream file{output, std::ios::binary};
	snowboy::WaveHeader header{};
	header.dwSamplesPerSec = cut.SampleRate();
	header.wBitsPerSample = cut.BitsPerSample();
	header.wChannels = cut.NumChannels();
	header.dwAvgBytesPerSec = header.dwSamplesPerSec * (header.wBitsPerSample / 8) * header.wChannels;
	header.dataChunkSize = res.size();
	header.chunkSize = sizeof(header) + res.size();
	file.write(reinterpret_cast<const char*>(&header), sizeof(header));
	file.write(res.data(), res.size());
	return 0;
}

bool parse_args(int argc, const char** argv, std::string& output, std::string& recording, std::string& lang) {
	lang = "en";
	option_parser parser;
	parser.option("--output", &output).set_shortname("-o").set_description("Output filename for the model");
	parser.option("--language", &output).set_shortname("-l").set_description("Language of the enrolled word");
	parser.option("--recording", &recording).set_shortname("-r").set_required(true).set_description("Recording to cut");
	bool print_help = false;
	parser.option("--help", &print_help).set_shortname("-h").set_description("Print help");
	std::vector<std::string> extra_args;
	try {
		extra_args = parser.parse(argc, argv);
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return false;
	}
	if (print_help) {
		parser.print_help(std::cout);
		return true;
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
