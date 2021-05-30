#include <execinfo.h>
#include <iostream>
#include <string>

namespace snowboy {
	std::string GetStackTrace() {
		std::string res{"\n[stack trace: ]\n"};
		void* buffer[50];
		auto nptrs = backtrace(buffer, sizeof(buffer) / sizeof(buffer[0]));
		auto symbols = backtrace_symbols(buffer, nptrs);

		for (int i = 0; i < nptrs; i++) {
			res += symbols[i];
			res += "\n";
		}

		free(symbols);

		return res;
	}

	void SnowboyAssertFailure(int line, const std::string& file, const std::string& func, const std::string& cond) {
		std::cerr << "ASSERT_FAILURE (" << func << "():" << file << ":" << line << ")\n"
				  << cond;
		std::cerr << GetStackTrace();
		std::abort();
	}
} // namespace snowboy
