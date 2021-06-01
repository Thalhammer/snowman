#include <dtw-lib.h>
#include <helper.h>
#include <matrix-wrapper.h>

const static auto root = detect_project_root();

TEST(DtwTest, RandomSelf) {
	unsigned int seed = 0;
	auto m = random_matrix(&seed);

	auto res = snowboy::DtwAlign(snowboy::cosine, m, m, nullptr);
	ASSERT_FLOAT_EQ(res, 0.49959436);

	std::vector<std::vector<size_t>> t;
	res = snowboy::DtwAlign(snowboy::cosine, m, m, &t);
	ASSERT_FLOAT_EQ(res, 0.49959436);
	ASSERT_EQ(t.size(), 39);
	for (size_t i = 0; i < t.size(); i++) {
		ASSERT_EQ(t[i].size(), 1);
		ASSERT_EQ(t[i][0], i);
	}
}

TEST(DtwTest, RandomTwice) {
	unsigned int seed = 0;
	auto m1 = random_matrix(&seed);
	auto m2 = random_matrix(&seed);

	auto res = snowboy::DtwAlign(snowboy::cosine, m1, m2, nullptr);
	ASSERT_FLOAT_EQ(res, 0.49965164);

	std::vector<std::vector<size_t>> t;
	res = snowboy::DtwAlign(snowboy::cosine, m1, m2, &t);
	ASSERT_FLOAT_EQ(res, 0.49965164);
	ASSERT_EQ(t.size(), 39);
	for (size_t i = 0; i < t.size(); i++) {
		ASSERT_EQ(t[i].size(), 1);
		EXPECT_EQ(t[i][0], 2);
	}

	res = snowboy::DtwAlign(snowboy::cosine, m2, m1, nullptr);
	ASSERT_FLOAT_EQ(res, 0.49963593);

	res = snowboy::DtwAlign(snowboy::cosine, m2, m1, &t);
	ASSERT_FLOAT_EQ(res, 0.49963593);
	ASSERT_EQ(t.size(), 20);
	for (size_t i = 0; i < t.size(); i++) {
		ASSERT_EQ(t[i].size(), 1);
		EXPECT_EQ(t[i][0], 35);
	}
}

TEST(DtwTest, Random2) {
	unsigned int seed = 42; // Perfectly random
	auto m1 = random_matrix(&seed);
	auto m2 = random_matrix(&seed);

	auto res = snowboy::DtwAlign(snowboy::cosine, m1, m2, nullptr);
	ASSERT_FLOAT_EQ(res, 0.49967793);

	std::vector<std::vector<size_t>> t;
	res = snowboy::DtwAlign(snowboy::cosine, m1, m2, &t);
	ASSERT_FLOAT_EQ(res, 0.49967793);
	ASSERT_EQ(t.size(), 45);
	for (size_t i = 0; i < t.size(); i++) {
		ASSERT_EQ(t[i].size(), 1);
		EXPECT_EQ(t[i][0], 5);
	}

	res = snowboy::DtwAlign(snowboy::cosine, m2, m1, nullptr);
	ASSERT_FLOAT_EQ(res, 0.4996832);

	res = snowboy::DtwAlign(snowboy::cosine, m2, m1, &t);
	ASSERT_FLOAT_EQ(res, 0.4996832);
	ASSERT_EQ(t.size(), 26);
	for (size_t i = 0; i < t.size(); i++) {
		ASSERT_EQ(t[i].size(), 1);
		EXPECT_EQ(t[i][0], 32);
	}
}
