#include <execinfo.h>
#include <iostream>
#include <snowboy-debug.h>
#include <string>

namespace snowboy {
    int global_snowboy_verbose_level = 0;

	std::string GetStackTrace() {
        std::string res{"\n[stack trace: ]\n"};
        void* buffer[50];
        auto nptrs = backtrace(buffer, sizeof(buffer)/sizeof(buffer[0]));
        auto symbols = backtrace_symbols(buffer, nptrs);

        for(int i=0; i<nptrs; i++) {
            res += symbols[i];
            res += "\n";
        }

        free(symbols);

        return res;
    }

	void SnowboyAssertFailure(int line, const std::string& file, const std::string& func, const std::string& cond) {
	    snowboy::MySnowboyLogMsg msg { line, file, func, snowboy::SnowboyLogType::ASSERT_FAIL, 0 };
	    msg << cond;
	}

	MySnowboyLogMsg::MySnowboyLogMsg(int line, const std::string& file, const std::string& function, const SnowboyLogType& type, int)
		: m_type{type}, m_stream{} {
		switch (type)
		{
		case SnowboyLogType::ASSERT_FAIL:
			m_stream << "ASSERT_FAILURE (";
			break;
		case SnowboyLogType::ERROR:
			m_stream << "ERROR (";
			break;
		case SnowboyLogType::WARNING:
			m_stream << "WARNING (";
			break;
		case SnowboyLogType::LOG:
			m_stream << "LOG (";
			break;
		case SnowboyLogType::VLOG:
			m_stream << "VLOG (";
			break;
		}
		m_stream << function << "():" << file << ":" << line << ") ";
	}

	MySnowboyLogMsg::~MySnowboyLogMsg() noexcept(false) {
		std::cout << m_stream.str();
		if (m_type == SnowboyLogType::ERROR)
		{
			m_stream << GetStackTrace();
			// TODO: This isnt normally allowed....
			throw std::runtime_error(m_stream.str());
		}
	}
} // namespace snowboy