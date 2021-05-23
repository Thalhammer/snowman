#include <snowboy-debug.h>
#include <snowboy-io.h>

namespace snowboy {
	std::string global_snowboy_offset_delimiter{1, char(1)};
	std::string global_snowboy_string_delimiter{","};

	void CheckToken(const char* token) {
		if (token == nullptr || *token == '\0') {
			SNOWBOY_ERROR() << "Token is empty";
			return;
		}
		auto ptr = token;
		while (*ptr != '\0') {
			if (isspace(*ptr)) {
				SNOWBOY_ERROR() << "Token contains space: \'" << token << "\'";
				return;
			}
			ptr++;
		}
	}

	void EncryptToken(std::string* token) {
		for (auto& c : *token) {
			if (c == 0x43) continue;
			auto x = c ^ 0x43;
			if (isspace(x)) continue;
			c = x;
		}
	}

	void ExpectOneOrTwoTokens(bool binary, const std::string& token1, const std::string& token2, std::istream* is) {
		std::string token;
		ReadToken(binary, &token, is);
		if (token == token1) {
			ExpectToken(binary, token2, is);
		} else if (token != token2) {
			SNOWBOY_ERROR() << "Expected token \"" << token1 << "\" or \"" << token2 << "\", got instead \"" << token << "\".";
		}
	}

	void ExpectToken(bool binary, const char* expected_token, std::istream* is) {
		CheckToken(expected_token);
		std::string token;
		if (!binary) {
			*is >> std::ws;
			*is >> token;
		} else {
			auto pos = is->tellg();
			auto c = is->get();
			if (c == 0 && is->get() == 'E') {
				*is >> token;
				EncryptToken(&token);
				CheckToken(token.c_str());
			} else {
				is->seekg(pos);
				*is >> token;
			}
		}
		if (!(*is)) {
			SNOWBOY_ERROR() << "Fail to read token in ExpectToken(), expecting token " << expected_token;
			return;
		}
		if (token != expected_token) {
			SNOWBOY_ERROR() << "Expected token \"" << expected_token << "\", got instead \"" << token << "\".";
			return;
		}
		is->get();
	}

	void ExpectToken(bool binary, const std::string& token, std::istream* is) {
		ExpectToken(binary, token.c_str(), is);
	}

	void ReadToken(bool binary, std::string* token, std::istream* is) {
		if (!binary) {
			*is >> std::ws;
			*is >> *token;
		} else {
			auto pos = is->tellg();
			auto c = is->get();
			if (c == 0 && is->get() == 'E') {
				*is >> *token;
				EncryptToken(token);
				CheckToken(token->c_str());
			} else {
				is->seekg(pos);
				*is >> *token;
			}
		}
		if (!(*is)) {
			SNOWBOY_ERROR() << "Fail to read token in ReadToken(), position " << is->tellg();
			return;
		}
		if (isspace(is->peek()) == 0) {
			auto ch = CharToString(is->peek());
			SNOWBOY_ERROR() << "Failed to read token in ReadToken(): expected space after token, got instead " << ch << " at position " << is->tellg();
			return;
		}
		is->get(); // Skip space
	}

	int PeekToken(bool binary, std::istream* is) {
		auto pos = is->tellg();
		std::string token;
		if (!binary) {
			*is >> std::ws;
			*is >> token;
		} else {
			auto c = is->get();
			if (c == 0 && is->get() == 'E') {
				*is >> token;
				EncryptToken(&token);
				CheckToken(token.c_str());
			} else {
				is->seekg(pos);
				*is >> token;
			}
		}
		is->seekg(pos);
		if (token.empty()) return -1;
		if (token[0] == '<')
			return token[1];
		else
			return token[0];
	}

	void WriteToken(bool binary, const char* token, std::ostream* os) {
		CheckToken(token);
		if (!binary) {
			*os << token << " ";
		} else {
			std::string stoken{token};
			EncryptToken(&stoken);
			CheckToken(stoken.c_str());
			os->put('\0');
			os->put('E');
			*os << stoken << " ";
		}
		if (!*os) {
			SNOWBOY_ERROR() << "Fail to write token in WriteToken().";
			return;
		}
	}

	void WriteToken(bool binary, const std::string& token, std::ostream* os) {
		WriteToken(binary, token.c_str(), os);
	}

	template <>
	void ReadBasicType<bool>(bool binary, bool* t, std::istream* is) {
		if (!binary) {
			*is >> std::ws;
		}
		auto c = is->peek();
		if (c == 'T') {
			*t = true;
		} else if (c == 'F') {
			*t = false;
		} else {
			SNOWBOY_ERROR() << "Fail to read <bool> in ReadBasicType(), file position is " << is->tellg() << ", next char is " << CharToString(c);
			return;
		}
		is->get();
	}

	template <>
	void ReadBasicType<float>(bool binary, float* t, std::istream* is) {
		if (!binary) {
			*is >> *t;
		} else {
			auto c = is->peek();
			if (c == sizeof(float)) {
				is->get();
				is->read(reinterpret_cast<char*>(t), sizeof(*t));
			} else {
				SNOWBOY_ERROR() << "Failed to read <float> type in ReadBasicType(): "
								   "expected 4, got "
								<< static_cast<int>(c) << " instead at position " << is->tellg();
				return;
			}
		}
		if (!*is) {
			SNOWBOY_ERROR() << "Fail to read <float> in ReadBasicType(), file position is " << is->tellg();
		}
	}

	template <>
	void ReadBasicType<int32_t>(bool binary, int32_t* t, std::istream* is) {
		if (!binary) {
			*is >> *t;
		} else {
			auto c = is->peek();
			if (c == sizeof(int32_t)) {
				is->get();
				is->read(reinterpret_cast<char*>(t), sizeof(*t));
			} else {
				SNOWBOY_ERROR() << "Failed to read <int32_t> type in ReadBasicType(): "
								   "expected 4, got "
								<< static_cast<int>(c) << " instead at position " << is->tellg();
				return;
			}
		}
		if (!*is) {
			SNOWBOY_ERROR() << "Fail to read <int> in ReadBasicType(), file position is " << is->tellg();
		}
	}

	template <>
	void ReadBasicType<int64_t>(bool binary, int64_t* t, std::istream* is) {
		if (!binary) {
			*is >> *t;
		} else {
			auto c = is->peek();
			if (c == sizeof(int64_t)) {
				is->get();
				is->read(reinterpret_cast<char*>(t), sizeof(*t));
			} else {
				SNOWBOY_ERROR() << "Failed to read <int64_t> type in ReadBasicType(): "
								   "expected 8, got "
								<< static_cast<int>(c) << " instead at position " << is->tellg();
				return;
			}
		}
		if (!*is) {
			SNOWBOY_ERROR() << "Fail to read <long> in ReadBasicType(), file position is " << is->tellg();
		}
	}

	template <>
	void WriteBasicType<bool>(bool binary, bool t, std::ostream* os) {
		os->put(t ? 'T' : 'F');
		if (!binary) os->put(' ');
		if (!*os) {
			SNOWBOY_ERROR() << "Fail to write <bool> type in WriteBasicType().";
		}
	}

	template <>
	void WriteBasicType<float>(bool binary, float t, std::ostream* os) {
		if (!binary) {
			*os << t << " ";
		} else {
			os->put(sizeof(t));
			os->write(reinterpret_cast<char*>(&t), sizeof(t));
		}
		if (!*os) {
			SNOWBOY_ERROR() << "Fail to write <float> type in WriteBasicType().";
		}
	}

	template <>
	void WriteBasicType<int32_t>(bool binary, int32_t t, std::ostream* os) {
		if (!binary) {
			*os << t << " ";
		} else {
			os->put(sizeof(t));
			os->write(reinterpret_cast<char*>(&t), sizeof(t));
		}
		if (!*os) {
			SNOWBOY_ERROR() << "Fail to write <int> type in WriteBasicType().";
		}
	}

	template <>
	void WriteBasicType<int64_t>(bool binary, int64_t t, std::ostream* os) {
		if (!binary) {
			*os << t << " ";
		} else {
			os->put(sizeof(t));
			os->write(reinterpret_cast<char*>(&t), sizeof(t));
		}
		if (!*os) {
			SNOWBOY_ERROR() << "Fail to write <long> type in WriteBasicType().";
		}
	}

	void ReadStringVector(bool binary, std::vector<std::string>* vector, std::istream* is) {
		if (binary) {
			// Note: this is not a todo, because it wasnt implemented in snowboy
			SNOWBOY_ERROR() << "ReadStringVector: binary mode has not been implemented";
			return;
		}
		std::string line;
		std::getline(*is, line);
		SplitStringToVector(line, global_snowboy_whitespace_set, vector);
	}

	// TODO: Seems to be identical to ReadStringVector (expect for the Trim call, which imho is missing in ReadStringVector....)
	// I have no idea whats up with that, but they are not used anywhere anyway so I also dont care atm
	void ReadStringVectorByLines(bool binary, std::vector<std::string>* vector, std::istream* is) {
		if (binary) {
			// Note: this is not a todo, because it wasnt implemented in snowboy
			SNOWBOY_ERROR() << "ReadStringVectorByLines: binary mode has not been implemented";
			return;
		}
		std::string line;
		std::getline(*is, line);
		Trim(&line);
		SplitStringToVector(line, global_snowboy_whitespace_set, vector);
	}

	template <>
	void ReadIntegerVector<int>(bool binary, std::vector<int>* data, std::istream* is) {
		if (!binary) {
			ExpectToken(binary, "[", is);
			*is >> std::ws;
			for (auto c = is->peek(); c != ']'; c = is->peek()) {
				int x;
				*is >> x >> std::ws;
				if (!*is) {
					SNOWBOY_ERROR() << "Fail to ReadIntegerVector.";
					return;
				}
				data->push_back(x);
			}
			is->get(); // read closing bracket
		} else {
			auto c = is->peek();
			if (c != sizeof(int)) {
				SNOWBOY_ERROR() << "Fail to read integer type in ReadIntegerVector(): expecting type of size "
								<< sizeof(int) << ", got instead " << static_cast<int>(c);
				return;
			}
			is->get();
			int size = -1;
			is->read(reinterpret_cast<char*>(&size), sizeof(size));
			if (!*is || size < 0) {
				SNOWBOY_ERROR() << "Fail to read integer type in ReadIntegerVector(): expecting vector size, got " << size;
				return;
			}
			data->resize(size);
			is->read(reinterpret_cast<char*>(data->data()), size * sizeof(int));
			if (!*is) {
				SNOWBOY_ERROR() << "Fail to ReadIntegerVector.";
				return;
			}
		}
	}

	template <>
	void WriteIntegerVector<int>(bool binary, const std::vector<int>& data, std::ostream* os) {
		if (!binary) {
			*os << "[ ";
			for (auto& e : data) {
				*os << e << " ";
			}
			*os << "]\n";
		} else {
			os->put(sizeof(int));
			int size = data.size();
			os->write(reinterpret_cast<const char*>(&size), sizeof(size));
			if (size != 0) os->write(reinterpret_cast<const char*>(data.data()), size * sizeof(int));
		}
		if (!*os) {
			SNOWBOY_ERROR() << "Fail to write integer vector in WriteIntegerVector().";
			return;
		}
	}

	Output::Output(const std::string& filename, bool binary) {
		auto pos = filename.find_first_of(global_snowboy_offset_delimiter);
		if (pos != std::string::npos) {
			SNOWBOY_ERROR() << "Filename contains offset delimiter.";
			return;
		}
		m_stream.open(filename, std::ios::out | std::ios::binary);
		if (!m_stream.is_open()) {
			SNOWBOY_ERROR() << "Failed to open output file \"" << filename << "\"";
			return;
		}
		if (binary) {
			m_stream.put(0x00);
			m_stream.put('B');
		}
	}

	std::ostream* Output::Stream() {
		return &m_stream;
	}

	Output::~Output() {}

	Input::Input(const std::string& filename) {
		std::string real_name;
		std::streampos pos = -1;
		ParseFilename(filename, &real_name, &pos);
		m_stream.open(real_name, std::ios::binary | std::ios::in);
		if (!m_stream.is_open()) {
			SNOWBOY_ERROR() << "Fail to open input file \"" << real_name << "\"";
			return;
		}
		if (pos != -1) {
			m_stream.seekg(pos);
			if (!m_stream) {
				SNOWBOY_ERROR() << "Fail to open input file \"" << real_name << "\" at offset " << pos;
				return;
			}
		}
		pos = m_stream.tellg();
		auto c = m_stream.get();
		if (c == 0x00 && m_stream.get() == 'B') {
			m_is_binary = true;
		} else {
			m_stream.seekg(pos);
			m_is_binary = false;
		}
	}

	std::istream* Input::Stream() {
		return &m_stream;
	}

	Input::~Input() {}

	void Input::ParseFilename(const std::string& filename, std::string* real_name, std::streampos* offset) const {
		std::vector<std::string> parts;
		SplitStringToVector(filename, global_snowboy_offset_delimiter, &parts);
		if (parts.size() == 1) {
			*real_name = parts[0];
			*offset = 0;
		} else if (parts.size() == 2) {
			*real_name = parts[0];
			*offset = ConvertStringToIntegerOrFloat<int>(parts[1]);
		} else {
			SNOWBOY_ERROR() << "Filename is empty or contains offset delimiter";
			return;
		}
	}
} // namespace snowboy
