#include <helper.h>
#include <iostream>
#include <snowboy-detect.h>
#include <pulseaudio.h>

const static auto root = detect_project_root();

namespace pa = pulseaudio;

std::vector<short> record_word(pa::simple_record_stream& audio_in) {
	snowboy::SnowboyVad vad{root + "resources/common.res"};
	std::vector<short> samples;
	std::vector<short> result;
	audio_in.read(samples);
	// Skip leading silence
	while (vad.RunVad(samples.data(), samples.size()) == -2) {
		std::cout << "S" << std::flush;
		audio_in.read(samples);
	}
	// Keep samples while there is voice
	result = samples;
	while (vad.RunVad(samples.data(), samples.size()) == 0) {
		std::cout << "A" << std::flush;
		audio_in.read(samples);
		result.insert(result.end(), samples.begin(), samples.end());
	}
	std::cout << "D\n" << std::flush;
	return result;
}

int main(int argc, const char** argv) try
{
	std::string output = "model.pmdl", language = "en";
	int64_t num_records = 3;
	option_parser parser;
	parser.option("--output", &output).set_shortname("-o").set_description("Output filename for the model");
	parser.option("--language", &language).set_shortname("-l").set_description("Language of the enrolled word");
	parser.option("--nrecs", &num_records).set_min(3).set_shortname("-n").set_required(true).set_description("Number of hotword samples to record for the model");
	parser.parse(argc, argv);

	snowboy::SnowboyPersonalEnroll enroll{root + "resources/pmdl/en/personal_enroll.res", "model.pmdl"};
	snowboy::SnowboyTemplateCut cut{root + "resources/pmdl/en/personal_enroll.res"};
	pa::simple_record_stream audio_in{"Microphone input", enroll.SampleRate(), enroll.NumChannels()};

	for(int64_t i=1; i<=num_records; i++) {
		std::cout << "[" << i << "/" << num_records << "] ";
		auto sample = record_word(audio_in);
		std::cout << "[" << i << "/" << num_records << "] Got sample with length = " << (static_cast<float>(sample.size())/16000.0f) << "s (" << sample.size() << " samples)" << std::endl;
		int new_size;
		if(cut.CutTemplate(sample.data(), sample.size(), sample.data(), &new_size) != 0)
			throw std::runtime_error("Failed to cut template");
		sample.resize(new_size);
		std::cout << "[" << i << "/" << num_records << "] length after cutting = " << (static_cast<float>(sample.size())/16000.0f) << "s (" << sample.size() << " samples)" << std::endl;
		auto res = enroll.RunEnrollment(sample.data(), sample.size());
		if (res != 0) {
			std::cerr << "Training failed with error " << res << std::endl;
			return -1;
		}
	}
	return 0;
} catch (const std::exception& e) {
	std::cerr << "Error: " << e.what() << std::endl;
	return -1;
}
