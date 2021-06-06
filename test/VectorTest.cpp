#include <helper.h>
#include <vector-wrapper.h>

using namespace snowboy;

TEST(VectorTest, DefaultConstruct) {
	Vector v;
	ASSERT_EQ(v.size(), 0);
	ASSERT_EQ(v.capacity(), 0);
	ASSERT_EQ(v.data(), nullptr);
}

TEST(VectorTest, ResizeFromEmptySetZero) {
	Vector v;
	auto size = rand() % 1000 + 100;
	v.Resize(size, MatrixResizeType::kSetZero);
	ASSERT_EQ(v.size(), size);
	ASSERT_GE(v.capacity(), size);
	ASSERT_NE(v.data(), nullptr);
	for (size_t i = 0; i < v.size(); i++) {
		ASSERT_EQ(v[i], 0.0f);
	}
}

TEST(VectorTest, ResizeFromEmptyUndefined) {
	Vector v;
	auto size = rand() % 1000 + 100;
	v.Resize(size, MatrixResizeType::kUndefined);
	ASSERT_EQ(v.size(), size);
	ASSERT_GE(v.capacity(), size);
	ASSERT_NE(v.data(), nullptr);
}

TEST(VectorTest, ResizeFromEmptyCopyData) {
	Vector v;
	auto size = rand() % 1000 + 100;
	v.Resize(size, MatrixResizeType::kCopyData);
	ASSERT_EQ(v.size(), size);
	ASSERT_GE(v.capacity(), size);
	ASSERT_NE(v.data(), nullptr);
	for (size_t i = 0; i < v.size(); i++) {
		ASSERT_EQ(v[i], 0.0f);
	}
}

TEST(VectorTest, ResizeEnlargeSetZero) {
	Vector v;
	auto size = rand() % 1000 + 100;
	v.Resize(size, MatrixResizeType::kUndefined);
	for (int i = 0; i < v.size(); i++)
		v[i] = i;
	v.Resize(size * 2, MatrixResizeType::kSetZero);
	ASSERT_EQ(v.size(), size * 2);
	ASSERT_GE(v.capacity(), size * 2);
	ASSERT_NE(v.data(), nullptr);
	for (size_t i = 0; i < v.size(); i++) {
		ASSERT_EQ(v[i], 0.0f);
	}
}

TEST(VectorTest, ResizeEnlargeUndefined) {
	Vector v;
	auto size = rand() % 1000 + 100;
	v.Resize(size, MatrixResizeType::kUndefined);
	for (int i = 0; i < v.size(); i++)
		v[i] = i;
	v.Resize(size * 2, MatrixResizeType::kUndefined);
	ASSERT_EQ(v.size(), size * 2);
	ASSERT_GE(v.capacity(), size * 2);
	ASSERT_NE(v.data(), nullptr);
}

TEST(VectorTest, ResizeEnlargeCopyData) {
	Vector v;
	auto size = rand() % 1000 + 100;
	v.Resize(size, MatrixResizeType::kUndefined);
	for (int i = 0; i < v.size(); i++)
		v[i] = i;
	v.Resize(size * 2, MatrixResizeType::kCopyData);
	ASSERT_EQ(v.size(), size * 2);
	ASSERT_GE(v.capacity(), size * 2);
	ASSERT_NE(v.data(), nullptr);
	for (size_t i = 0; i < size; i++) {
		ASSERT_EQ(v[i], i);
	}
	for (size_t i = size; i < v.size(); i++) {
		ASSERT_EQ(v[i], 0.0f);
	}
}

TEST(VectorTest, ResizeReduceSetZero) {
	Vector v;
	auto size = rand() % 1000 + 100;
	v.Resize(size, MatrixResizeType::kUndefined);
	for (int i = 0; i < v.size(); i++)
		v[i] = i;
	v.Resize(size / 2, MatrixResizeType::kSetZero);
	ASSERT_EQ(v.size(), size / 2);
	ASSERT_GE(v.capacity(), size);
	ASSERT_NE(v.data(), nullptr);
	for (size_t i = 0; i < v.size(); i++) {
		ASSERT_EQ(v[i], 0.0f);
	}
}

TEST(VectorTest, ResizeReduceUndefined) {
	Vector v;
	auto size = rand() % 1000 + 100;
	v.Resize(size, MatrixResizeType::kUndefined);
	for (int i = 0; i < v.size(); i++)
		v[i] = i;
	v.Resize(size / 2, MatrixResizeType::kUndefined);
	ASSERT_EQ(v.size(), size / 2);
	ASSERT_GE(v.capacity(), size);
	ASSERT_NE(v.data(), nullptr);
}

TEST(VectorTest, ResizeReduceCopyData) {
	Vector v;
	auto size = rand() % 1000 + 100;
	v.Resize(size, MatrixResizeType::kUndefined);
	for (int i = 0; i < v.size(); i++)
		v[i] = i;
	v.Resize(size / 2, MatrixResizeType::kCopyData);
	ASSERT_EQ(v.size(), size / 2);
	ASSERT_GE(v.capacity(), size);
	ASSERT_NE(v.data(), nullptr);
	for (size_t i = 0; i < size / 2; i++) {
		ASSERT_EQ(v[i], i);
	}
}

TEST(VectorTest, Copy) {
	Vector v;
	auto size = rand() % 1000 + 100;
	v.Resize(size, MatrixResizeType::kUndefined);
	for (int i = 0; i < v.size(); i++)
		v[i] = i;
	ASSERT_EQ(v.size(), size);
	ASSERT_GE(v.capacity(), size);
	ASSERT_NE(v.data(), nullptr);

	{
		Vector v2{v};
		ASSERT_EQ(v2.size(), size);
		ASSERT_GE(v2.capacity(), size);
		ASSERT_NE(v2.data(), nullptr);
		ASSERT_NE(v2.data(), v.data());
		for (int i = 0; i < v2.size(); i++)
			ASSERT_EQ(v2[i], i);
	}
	{
		Vector v2;
		v2 = v;
		ASSERT_EQ(v2.size(), size);
		ASSERT_GE(v2.capacity(), size);
		ASSERT_NE(v2.data(), nullptr);
		ASSERT_NE(v2.data(), v.data());
		for (int i = 0; i < v2.size(); i++)
			ASSERT_EQ(v2[i], i);
	}
}

TEST(VectorTest, MoveConstruct) {
	Vector v;
	auto size = rand() % 1000 + 100;
	v.Resize(size, MatrixResizeType::kUndefined);
	for (int i = 0; i < v.size(); i++)
		v[i] = i;
	ASSERT_EQ(v.size(), size);
	ASSERT_GE(v.capacity(), size);
	ASSERT_NE(v.data(), nullptr);

	{
		auto old_data = v.data();
		Vector v2{std::move(v)};
		ASSERT_EQ(v2.size(), size);
		ASSERT_GE(v2.capacity(), size);
		ASSERT_EQ(v2.data(), old_data);
		for (int i = 0; i < v2.size(); i++)
			ASSERT_EQ(v2[i], i);

		ASSERT_EQ(v.size(), 0);
		ASSERT_EQ(v.capacity(), 0);
		ASSERT_EQ(v.data(), nullptr);
	}
}

TEST(VectorTest, MoveAssign) {
	Vector v;
	auto size = rand() % 1000 + 100;
	v.Resize(size, MatrixResizeType::kUndefined);
	for (int i = 0; i < v.size(); i++)
		v[i] = i;
	ASSERT_EQ(v.size(), size);
	ASSERT_GE(v.capacity(), size);
	ASSERT_NE(v.data(), nullptr);

	{
		auto old_data = v.data();
		Vector v2;
		v2 = std::move(v);
		ASSERT_EQ(v2.size(), size);
		ASSERT_GE(v2.capacity(), size);
		ASSERT_EQ(v2.data(), old_data);
		for (int i = 0; i < v2.size(); i++)
			ASSERT_EQ(v2[i], i);

		ASSERT_EQ(v.size(), 0);
		ASSERT_EQ(v.capacity(), 0);
		ASSERT_EQ(v.data(), nullptr);
	}
}
