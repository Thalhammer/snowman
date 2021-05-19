#include <framer-stream.h>
#include <fstream>
#include <helper.h>
#include <matrix-wrapper.h>
#include <pipeline-personal-enroll.h>
#include <snowboy-detect.h>
#include <template-container.h>
#include <template-enroll-stream.h>

const static auto root = detect_project_root();

TEST(CutTest, CutTemplate1) {
	snowboy::SnowboyTemplateCut cut{root + "resources/pmdl/en/personal_enroll.res"};
	auto data = read_sample_file_as_string(root + "audio_samples/hotword1.wav");

	auto res = cut.CutTemplate(data);
	ASSERT_EQ(res.size(), 30240);
	ASSERT_EQ("6A:77:2F:13:2E:32:F7:BD:82:19:78:42:D3:5E:63:FD", md5sum(res));
}

TEST(CutTest, CutTemplateShort) {
	snowboy::SnowboyTemplateCut cut{root + "resources/pmdl/en/personal_enroll.res"};
	auto data = read_sample_file(root + "audio_samples/hotword1.wav");

	int new_size = 0;
	auto res = cut.CutTemplate(data.data(), data.size(), data.data(), &new_size);
	ASSERT_EQ(res, 0);
	ASSERT_EQ(new_size, 15120);
	ASSERT_EQ("6A:77:2F:13:2E:32:F7:BD:82:19:78:42:D3:5E:63:FD", md5sum(data.data(), new_size * sizeof(int16_t)));
}

TEST(CutTest, CutTemplate2) {
	snowboy::SnowboyTemplateCut cut{root + "resources/pmdl/en/personal_enroll.res"};
	auto data = read_sample_file_as_string(root + "audio_samples/hotword2.wav");

	auto res = cut.CutTemplate(data);
	ASSERT_EQ(res.size(), 26080);
	ASSERT_EQ("D4:ED:48:FA:C3:E0:DA:14:0F:A6:38:C3:D3:92:08:CD", md5sum(res));
}

TEST(CutTest, CutTemplate3) {
	snowboy::SnowboyTemplateCut cut{root + "resources/pmdl/en/personal_enroll.res"};
	auto data = read_sample_file_as_string(root + "audio_samples/hotword3.wav");

	auto res = cut.CutTemplate(data);
	ASSERT_EQ(res.size(), 26080);
	ASSERT_EQ("DE:F3:BD:6F:10:5D:EB:5F:01:BA:92:3E:1E:B6:4A:FC", md5sum(res));
}
