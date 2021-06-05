#include <helper.h>
#include <matrix-wrapper.h>
#include <snowboy-detect.h>
#include <vad-lib.h>
#include <vector-wrapper.h>

const static std::map<std::string, int> sample_map{
	{"hotword1.wav", 1},
	{"hotword2.wav", 1},
	{"hotword3.wav", 1},
	{"hotword3_fail.wav", 0},
	{"noise1.wav", -2},
	{"noise2.wav", -2},
	{"noise3.wav", -2},
	{"sample1.wav", 1},
	{"snowboy.wav", 1},
	{"alma1.wav", -2}};

const static std::map<std::string, int> sample_map_pmdl{
	{"hotword1.wav", 0},
	{"hotword2.wav", 0},
	{"hotword3.wav", 0},
	{"hotword3_fail.wav", 0},
	{"noise1.wav", -2},
	{"noise2.wav", -2},
	{"noise3.wav", -2},
	{"sample1.wav", 0},
	{"snowboy.wav", 0},
	{"alma1.wav", 1}};

const static std::string model_map[]{
	"Alexa.pmdl",
	"Alma.pmdl",
	"Ava.pmdl",
	"computer.umdl",
	"hey_extreme.umdl",
	"J.A.R.V.I.S.pmdl",
	"jarvis.pmdl",
	"jarvis.umdl",
	"Licht_An.pmdl",
	"Licht_Aus.pmdl",
	"Nachtlicht_an.pmdl",
	"neoya.umdl",
	"OK_Haus.pmdl",
	"smart_mirror.pmdl",
	"smart_mirror.umdl",
	"snowboy.umdl",
	"subex.umdl",
	"view_glass.umdl"};

const static auto root = detect_project_root();

TEST(ClassifyTest, ClassifySamples) {
	snowboy::Vector::ResetAllocStats();
	snowboy::Matrix::ResetAllocStats();
	bool skipped_all = true;
	for (auto& e : sample_map) {
		if (!file_exists(root + "audio_samples/" + e.first)) {
			GTEST_WARN("Skiping %s because audio file is missing!", e.first.c_str());
			continue;
		}
		skipped_all = false;
		auto data = read_sample_file(root + "audio_samples/" + e.first);
		// Initializes Snowboy detector.
		snowboy::SnowboyDetect detector(root + "resources/common.res", root + "resources/models/snowboy.umdl");
		detector.SetSensitivity("0.5");
		detector.SetAudioGain(1.0);
		detector.ApplyFrontend(false);

		int result = detector.RunDetection(data.data(), data.size());
		EXPECT_EQ(result, e.second) << "Failed to correctly classify sample " << e.first;
	}
	ASSERT_FALSE(skipped_all);
	snowboy::Vector::PrintAllocStats(std::cout);
	std::cout << "\n";
	snowboy::Matrix::PrintAllocStats(std::cout);
	std::cout << "\n";
}

TEST(ClassifyTest, ClassifySamplesChunked) {
	bool skipped_all = true;
	for (auto& e : sample_map) {
		if (!file_exists(root + "audio_samples/" + e.first)) {
			GTEST_WARN("Skiping %s because audio file is missing!", e.first.c_str());
			continue;
		}
		skipped_all = false;
		auto data = read_sample_file(root + "audio_samples/" + e.first);
		// Initializes Snowboy detector.
		snowboy::SnowboyDetect detector(root + "resources/common.res", root + "resources/models/snowboy.umdl");
		detector.SetSensitivity("0.5");
		detector.SetAudioGain(1.0);
		detector.ApplyFrontend(false);

		int result = -3;
		const auto chunksize = 4096;
		for (size_t i = 0; i < data.size(); i += chunksize) {
			auto len = std::min<int>(chunksize, data.size() - i);
			result = std::max(result, detector.RunDetection(data.data() + i, len, len != chunksize));
		}
		if (e.second > 0)
			EXPECT_EQ(result, e.second) << "Failed to correctly classify sample " << e.first;
		else {
			EXPECT_LE(result, 0) << "Failed to correctly classify sample " << e.first;
			EXPECT_GE(result, -2) << "Failed to correctly classify sample " << e.first;
		}
	}
	ASSERT_FALSE(skipped_all);
}

TEST(ClassifyTest, ClassifySamplesAlma) {
	auto model_exists = file_exists(root + "resources/models/Alma.pmdl");
	if (!model_exists) {
		GTEST_WARN("Missing private model, this does not mean, that private models dont work, just that my model is not present!");
		return;
	}

	snowboy::Vector::ResetAllocStats();
	snowboy::Matrix::ResetAllocStats();
	bool skipped_all = true;
	for (auto& e : sample_map_pmdl) {
		if (!file_exists(root + "audio_samples/" + e.first)) {
			GTEST_WARN("Skiping %s because audio file is missing!", e.first.c_str());
			continue;
		}
		skipped_all = false;
		auto data = read_sample_file(root + "audio_samples/" + e.first);
		// Initializes Snowboy detector.
		snowboy::SnowboyDetect detector(root + "resources/common.res", root + "resources/models/Alma.pmdl");
		detector.SetSensitivity("0.5");
		detector.SetAudioGain(1.0);
		detector.ApplyFrontend(false);

		int result = detector.RunDetection(data.data(), data.size());
		EXPECT_EQ(result, e.second) << "Failed to correctly classify sample " << e.first;
	}
	ASSERT_FALSE(skipped_all);
	snowboy::Vector::PrintAllocStats(std::cout);
	std::cout << "\n";
	snowboy::Matrix::PrintAllocStats(std::cout);
	std::cout << "\n";
}

TEST(ClassifyTest, ClassifySamplesAlmaChunked) {
	auto model_exists = file_exists(root + "resources/models/Alma.pmdl");
	if (!model_exists) {
		GTEST_WARN("Missing private model, this does not mean, that private models dont work, just that my model is not present!");
		return;
	}

	bool skipped_all = true;
	for (auto& e : sample_map_pmdl) {
		if (!file_exists(root + "audio_samples/" + e.first)) {
			GTEST_WARN("Skiping %s because audio file is missing!", e.first.c_str());
			continue;
		}
		skipped_all = false;
		auto data = read_sample_file(root + "audio_samples/" + e.first);
		// Initializes Snowboy detector.
		snowboy::SnowboyDetect detector(root + "resources/common.res", root + "resources/models/Alma.pmdl");
		detector.SetSensitivity("0.5");
		detector.SetAudioGain(1.0);
		detector.ApplyFrontend(false);

		int result = -3;
		const int chunksize = 4096;
		for (size_t i = 0; i < data.size(); i += chunksize) {
			auto len = std::min<int>(chunksize, data.size() - i);
			result = std::max(result, detector.RunDetection(data.data() + i, len, len != chunksize));
		}
		if (e.second > 0)
			EXPECT_EQ(result, e.second) << "Failed to correctly classify sample " << e.first;
		else {
			EXPECT_LE(result, 0) << "Failed to correctly classify sample " << e.first;
			EXPECT_GE(result, -2) << "Failed to correctly classify sample " << e.first;
		}
	}
	ASSERT_FALSE(skipped_all);
}

TEST(ClassifyTest, ClassifySamplesReset) {
	snowboy::SnowboyDetect detector(root + "resources/common.res", root + "resources/models/snowboy.umdl");
	detector.SetSensitivity("0.5");
	detector.SetAudioGain(1.0);
	detector.ApplyFrontend(false);

	bool skipped_all = true;
	for (auto& e : sample_map) {
		if (!file_exists(root + "audio_samples/" + e.first)) {
			GTEST_WARN("Skiping %s because audio file is missing!", e.first.c_str());
			continue;
		}
		skipped_all = false;
		auto data = read_sample_file(root + "audio_samples/" + e.first);
		for (size_t i = 0; i < 20; i++) {
			int result = detector.RunDetection(data.data(), data.size());
			EXPECT_EQ(result, e.second) << "Failed to correctly classify sample " << e.first;
			ASSERT_TRUE(detector.Reset());
		}
	}
	ASSERT_FALSE(skipped_all);
}

TEST(ClassifyTest, LoadModels) {
	bool skipped_all = true;
	for (auto& e : model_map) {
		if (!file_exists(root + "resources/models/" + e)) {
			GTEST_WARN("Skiping %s because model is missing!", e.c_str());
			continue;
		}
		skipped_all = false;
		snowboy::SnowboyDetect detector(root + "resources/common.res", root + "resources/models/" + e);
		detector.SetAudioGain(1.0);
		detector.ApplyFrontend(false);

		for (auto& e : sample_map) {
			if (!file_exists(root + "audio_samples/" + e.first)) {
				GTEST_WARN("Skiping %s because audio file is missing!", e.first.c_str());
				continue;
			}
			auto data = read_sample_file(root + "audio_samples/" + e.first);
			detector.RunDetection(data.data(), data.size());
			//ASSERT_EQ(result, e.second) << "Failed to correctly classify sample " << e.first;
			ASSERT_TRUE(detector.Reset());
		}
	}
	ASSERT_FALSE(skipped_all);
}
