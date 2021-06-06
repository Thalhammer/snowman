#include <helper.h>
#include <iostream>
#include <snowboy-detect.h>
#include <pulseaudio.h>

const static auto root = detect_project_root();

namespace pa = pulseaudio;

int main(int argc, const char** argv) try
{
	std::string model = root + "resources/models/snowboy.umdl";
	if(argc > 1) model = argv[1];
	pa::simple_record_stream audio_in{"Microphone input"};
	pa::simple_playback_stream audio_out{"Ding"};

	snowboy::SnowboyDetect detector(root + "resources/common.res", model);
	detector.SetSensitivity("0.3");
	detector.SetAudioGain(1.0);
	detector.ApplyFrontend(true);

	auto ding = read_sample_file(root + "resources/dong.wav");
	std::vector<short> samples;
	while (true) {
		audio_in.read(samples);
		auto s = detector.RunDetection(samples.data(), samples.size(), false);
		//std::cout << "\r   \r" << s << std::flush;
		if (s > 0) {
			std::cout << "a " << s << std::endl;
			audio_out.write(ding);
		}
	}
	return 0;
} catch (const std::exception& e) {
	std::cerr << "Error: " << e.what() << std::endl;
	return -1;
}
