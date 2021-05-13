#pragma once
#include <fstream>
#include <snowboy-utils.h>
#include <vector>

namespace snowboy {
	extern std::string global_snowboy_offset_delimiter;
	extern std::string global_snowboy_string_delimiter;

	void CheckToken(const char* token);
	void EncryptToken(std::string* token);
	void ExpectOneOrTwoTokens(bool binary, const std::string& token1, const std::string& token2, std::istream* is);
	void ExpectToken(bool binary, const char* token, std::istream* is);
	void ExpectToken(bool binary, const std::string& token, std::istream* is);
	void ReadToken(bool binary, std::string* token, std::istream* is);
	int PeekToken(bool binary, std::istream* is);
	void WriteToken(bool binary, const char* token, std::ostream* os);
	void WriteToken(bool binary, const std::string& token, std::ostream* os);
	template <class T>
	void ReadBasicType(bool binary, T* t, std::istream* is);
	template <class T>
	void WriteBasicType(bool binary, T t, std::ostream* os);
	void ReadStringVector(bool binary, std::vector<std::string>* vector, std::istream* is);
	void ReadStringVectorByLines(bool binary, std::vector<std::string>* vector, std::istream* is);
	template <typename T>
	void ReadIntegerVector(bool binary, std::vector<T>* data, std::istream* is);
	template <typename T>
	void WriteIntegerVector(bool binary, const std::vector<T>& data, std::ostream* os);
	template <>
	void WriteIntegerVector<int>(bool binary, const std::vector<int>& data, std::ostream* os);

	class Output {
		std::ofstream m_stream;

	public:
		Output(const std::string& filename, bool binary);
		std::ostream* Stream();
		~Output();
	};

	class Input {
		std::ifstream m_stream;
		bool m_is_binary;

	public:
		Input(const std::string& filename);
		std::istream* Stream();
		~Input();

		bool is_binary() const noexcept { return m_is_binary; }

		void ParseFilename(const std::string& filename, std::string* real_name, std::streampos* offset) const;
	};
} // namespace snowboy