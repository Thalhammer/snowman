#pragma once
#include <array>
#include <exception>
#include <vector>

namespace snowboy {
	class snowboy_exception : public std::runtime_error {
		constexpr static size_t num_backtrace_frames = 20;

		std::array<void*, num_backtrace_frames> m_backtrace;
		void capture_backtrace() noexcept;

	public:
		snowboy_exception(const char* what_arg);
		snowboy_exception(const std::string& what_arg);
		snowboy_exception(const snowboy_exception& other) noexcept;
		snowboy_exception& operator=(const snowboy_exception& other) noexcept;

		std::vector<std::string> backtrace_symbols() const;
	};
} // namespace snowboy
