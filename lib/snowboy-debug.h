#pragma once
#include <string>

namespace snowboy {
	void SnowboyAssertFailure(int line, const std::string& file, const std::string& func, const std::string& cond);
} // namespace snowboy

#ifndef NDEBUG
#define SNOWBOY_ASSERT(cond)                                                          \
	do                                                                                \
	{                                                                                 \
		if (cond)                                                                     \
			(void)0;                                                                  \
		else                                                                          \
			::snowboy::SnowboyAssertFailure(__LINE__, __FILE__, __FUNCTION__, #cond); \
	} while (0)
#else
#define SNOWBOY_ASSERT(cond) (void)0
#endif
