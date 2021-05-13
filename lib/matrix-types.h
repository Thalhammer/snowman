#pragma once
namespace snowboy {
	enum class MatrixResizeType {
		kSetZero,
		kUndefined,
		kCopyData
	};
	enum class MatrixTransposeType {
		kTrans = 0x70,
		kNoTrans = 0x6f
	};
} // namespace snowboy