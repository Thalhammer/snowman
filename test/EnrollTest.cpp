#include <framer-stream.h>
#include <fstream>
#include <helper.h>
#include <inspector.h>
#include <matrix-wrapper.h>
#include <pipeline-personal-enroll.h>
#include <snowboy-detect.h>
#include <template-container.h>
#include <template-enroll-stream.h>

const static auto root = detect_project_root();

TEST(EnrollTest, PersonalEnroll) {
	{
		std::ofstream t{"temp_enroll_model.pmdl", std::ios::binary | std::ios::trunc};
	}
	snowboy::SnowboyPersonalEnroll enroll{root + "resources/pmdl/en/personal_enroll.res", "temp_enroll_model.pmdl"};
	std::vector<std::string> recordings = {
		"record1.wav.cut",
		"record1.wav.cut",
		"record1.wav.cut"};
	for (auto& e : recordings) {
		auto data = read_sample_file(root + "audio_samples/" + e, true);
		std::string str_data;
		str_data.resize(data.size() * 2);
		memcpy(const_cast<char*>(str_data.data()), data.data(), str_data.size());
		ASSERT_TRUE(str_data.size() > 0);
		auto res = enroll.RunEnrollment(str_data);
		ASSERT_EQ(res, 0);
	}
	ASSERT_TRUE(file_exists("temp_enroll_model.pmdl"));
	auto stream = snowboy::testing::Inspector::PipelinePersonalEnroll_GetTemplateEnrollStream(
		snowboy::testing::Inspector::SnowboyPersonalEnroll_GetEnrollPipeline(enroll));
	int64_t h = hash(stream->field_x38.m_templates.front());
	ASSERT_LE(abs(h - 928553), 2);
}

TEST(EnrollTest, PersonalEnroll2) {
	{
		std::ofstream t{"temp_enroll_model.pmdl", std::ios::binary | std::ios::trunc};
	}
	snowboy::SnowboyPersonalEnroll enroll{root + "resources/pmdl/en/personal_enroll.res", "temp_enroll_model.pmdl"};
	std::vector<std::string> recordings = {
		"record1.wav.cut",
		"record2.wav.cut",
		"record3.wav.cut"};
	for (auto& e : recordings) {
		auto data = read_sample_file(root + "audio_samples/" + e, true);
		std::string str_data;
		str_data.resize(data.size() * 2);
		memcpy(const_cast<char*>(str_data.data()), data.data(), str_data.size());
		ASSERT_TRUE(str_data.size() > 0);
		auto res = enroll.RunEnrollment(str_data);
		ASSERT_EQ(res, 0);
	}
	ASSERT_TRUE(file_exists("temp_enroll_model.pmdl"));
	auto stream = snowboy::testing::Inspector::PipelinePersonalEnroll_GetTemplateEnrollStream(
		snowboy::testing::Inspector::SnowboyPersonalEnroll_GetEnrollPipeline(enroll));
	int64_t h = hash(stream->field_x38.m_templates.front());
	ASSERT_LE(abs(h - 928522), 2);
}
