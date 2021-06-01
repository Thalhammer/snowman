#include <execinfo.h>
#include <memory>
#include <snowboy-error.h>

namespace snowboy {
	void snowboy_exception::capture_backtrace() noexcept {
		auto res = backtrace(m_backtrace.data(), m_backtrace.size());
		for (size_t i = res; i < m_backtrace.size(); i++)
			m_backtrace[i] = nullptr;
	}

	snowboy_exception::snowboy_exception(const char* what_arg)
		: std::runtime_error(what_arg) {
		capture_backtrace();
	}

	snowboy_exception::snowboy_exception(const std::string& what_arg)
		: std::runtime_error(what_arg) {
		capture_backtrace();
	}

	snowboy_exception::snowboy_exception(const snowboy_exception& other) noexcept
		: std::runtime_error(*this), m_backtrace(other.m_backtrace) {}

	snowboy_exception& snowboy_exception::operator=(const snowboy_exception& other) noexcept {
		runtime_error::operator=(other);
		m_backtrace = other.m_backtrace;
		return *this;
	}

	std::vector<std::string> snowboy_exception::backtrace_symbols() const {
		size_t nptrs = 0;
		for (; nptrs < m_backtrace.size() && m_backtrace[nptrs] != nullptr; nptrs++)
			;

		auto symbols = ::backtrace_symbols(m_backtrace.data(), nptrs);

		std::vector<std::string> res;
		try {
			for (size_t i = 0; i < nptrs; i++) {
				res.push_back(symbols[i]);
			}
			free(symbols);
		} catch (...) {
			free(symbols);
			throw;
		}
		return res;
	}

} // namespace snowboy
