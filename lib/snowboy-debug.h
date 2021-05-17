#pragma once
#include <sstream>

namespace snowboy {
	extern int global_snowboy_verbose_level;
	std::string GetStackTrace();
	void SnowboyAssertFailure(int line, const std::string& file, const std::string& func, const std::string& cond);
	enum class SnowboyLogType {
		ASSERT_FAIL = -1,
		ERROR = 0,
		WARNING,
		LOG,
		VLOG
	};
	struct SnowboyLogMsg {
		SnowboyLogType m_type;
		std::stringstream m_stream;
		SnowboyLogMsg(int line, const std::string&, const std::string&, const SnowboyLogType& type, int);
		~SnowboyLogMsg() noexcept(false);

		template <typename T>
		SnowboyLogMsg& operator<<(T&& val) {
			m_stream << val;
			return *this;
		}
	};
} // namespace snowboy

#define SNOWBOY_ERROR() \
	snowboy::SnowboyLogMsg { __LINE__, __FILE__, __FUNCTION__, snowboy::SnowboyLogType::ERROR, 0 }
#define SNOWBOY_WARNING() \
	snowboy::SnowboyLogMsg { __LINE__, __FILE__, __FUNCTION__, snowboy::SnowboyLogType::WARNING, 0 }
#define SNOWBOY_LOG() \
	snowboy::SnowboyLogMsg { __LINE__, __FILE__, __FUNCTION__, snowboy::SnowboyLogType::LOG, 0 }
#define SNOWBOY_VLOG() \
	snowboy::SnowboyLogMsg { __LINE__, __FILE__, __FUNCTION__, snowboy::SnowboyLogType::VLOG, 0 }

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