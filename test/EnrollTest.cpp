#include <framer-stream.h>
#include <fstream>
#include <helper.h>
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
	for (auto& e : enroll.enroll_pipeline_->m_templateEnrollStream->field_x38.m_templates) {
		std::cout << e.m_rows << "x" << e.m_cols << " hash=" << hash(e) << std::endl;
	}
	ASSERT_TRUE(file_exists("temp_enroll_model.pmdl"));
    ASSERT_EQ(hash(enroll.enroll_pipeline_->m_templateEnrollStream->field_x38.m_templates.front()), 928553);
    // TODO: I would really like to do a md5 of the file instead but due to rounding errors (and dithering) thats not an option
	// auto hash = md5sum_file("temp_enroll_model.pmdl");
	// ASSERT_EQ(hash, "55:F2:DA:B4:E7:6E:B2:68:6F:DA:6E:0E:0D:13:39:F1");
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
	for (auto& e : enroll.enroll_pipeline_->m_templateEnrollStream->field_x38.m_templates) {
		std::cout << e.m_rows << "x" << e.m_cols << " hash=" << hash(e) << std::endl;
	}
	ASSERT_TRUE(file_exists("temp_enroll_model.pmdl"));
    ASSERT_EQ(hash(enroll.enroll_pipeline_->m_templateEnrollStream->field_x38.m_templates.front()), 928522);
    // TODO: I would really like to do a md5 of the file instead but due to rounding errors (and dithering) thats not an option
	// auto hash = md5sum_file("temp_enroll_model.pmdl");
	// ASSERT_EQ(hash, "44:F9:AE:8E:CB:4A:E0:36:C6:FC:7D:9A:DA:C3:67:0F");
}