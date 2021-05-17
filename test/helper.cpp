#include <execinfo.h>
#include <fstream>
#include <helper.h>
#include <malloc.h>
#include <openssl/crypto.h>
#include <openssl/md5.h>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>

std::vector<short> read_sample_file(const std::string& filename, bool treat_wave) {

	struct wavHeader { //44 byte HEADER only
		char RIFF[4];
		int RIFFsize;
		char fmt[8];
		int fmtSize;
		short fmtTag;
		short nchan;
		int fs;
		int avgBps;
		short nBlockAlign;
		short bps;
		char data[4];
		int datasize;
	};

	auto readWavHeader = [](wavHeader* wavhdr, FILE* fi) {
		char* tag = (char*)wavhdr;
		if (fread(wavhdr, 1, 34, fi) != 34) throw std::runtime_error("fread failed");
		if (tag[0] != 'R' || tag[1] != 'I' || tag[2] != 'F' || tag[3] != 'F')
		{
			throw std::runtime_error("not a riff file");
		}
		if (wavhdr->fmtTag != 1)
		{
			throw std::runtime_error("WAV file has encoded data or it is WAVEFORMATEXTENSIBLE");
		}
		if (wavhdr->fmtSize == 14)
		{
			wavhdr->bps = 16;
		}
		if (wavhdr->fmtSize >= 16)
		{
			if (fread(&wavhdr->bps, 1, 2, fi) != 2) throw std::runtime_error("fread failed");
		}
		if (wavhdr->fmtSize == 18)
		{
			short lixo;
			if (fread(&lixo, 1, 2, fi) != 2) throw std::runtime_error("fread failed");
		}
		tag += 36;
		if (fread(tag, 1, 4, fi) != 4) throw std::runtime_error("fread failed");
		while (tag[0] != 'd' || tag[1] != 'a' || tag[2] != 't' || tag[3] != 'a')
		{
			if (fread(tag, 1, 4, fi) != 4) throw std::runtime_error("fread failed");
			if (ftell(fi) >= long(wavhdr->RIFFsize))
			{
				fclose(fi);
				perror("Bad WAV header !");
			}
		}
		if (fread(&wavhdr->datasize, 1, 4, fi) != 4) throw std::runtime_error("fread failed");
	};

	bool isWav = treat_wave || (filename.size() > 4 && filename.rfind(".wav") == filename.size() - 4);

	FILE* f = fopen(filename.c_str(), "rb");
	if (f == NULL)
	{
		throw std::runtime_error("Error opening file");
		return {};
	}

	std::vector<short> res;
	try {
		if (isWav)
		{
			wavHeader wavhdr{};
			readWavHeader(&wavhdr, f);
			res.resize(wavhdr.datasize / 2);
		} else
		{
			fseek(f, 0, SEEK_END);
			res.resize(ftell(f) / 2);
			rewind(f);
		}
		auto read = fread(res.data(), 1, res.size() * 2, f);
		while (read != res.size() * 2) {
			auto r = fread(res.data() + read, res.size() * 2 - read, 1, f);
			if (r <= 0) throw std::runtime_error("fread failed: wanted=" + std::to_string(res.size() * 2) + " got=" + std::to_string(read));
			read += r;
		}
	} catch (...) {
		fclose(f);
		throw;
	}
	fclose(f);
	return res;
}

std::string read_sample_file_as_string(const std::string& filename, bool treat_wave) {
	auto data = read_sample_file(filename, treat_wave);
	std::string str_data;
	str_data.resize(data.size() * 2);
	memcpy(const_cast<char*>(str_data.data()), data.data(), str_data.size());
	return str_data;
}

bool file_exists(const std::string& name) {
	struct stat buffer;
	return stat(name.c_str(), &buffer) == 0;
}

std::string detect_project_root() {
	std::string prefix;
	for (int i = 0; i < 5; i++) {
		if (file_exists(prefix + "./.git")) return prefix;
		prefix += "../";
	}
	return "";
}

std::string read_file(const std::string& file) {
	std::ifstream f{file, std::ios::binary};
	std::stringstream content;
	content << f.rdbuf();
	return content.str();
}

std::string md5sum(const std::string& data) {
	unsigned char hash[MD5_DIGEST_LENGTH];
	MD5(reinterpret_cast<const unsigned char*>(data.data()), data.size(), hash);
	auto ptr = OPENSSL_buf2hexstr(hash, MD5_DIGEST_LENGTH);
	std::string res = ptr;
	OPENSSL_free(ptr);
	return res;
}

std::string md5sum_file(const std::string& file) {
	auto content = read_file(file);
	return md5sum(content);
}

MemoryChecker::snapshot MemoryChecker::g_global{};

void MemoryChecker::stacktrace::capture() {
	auto res = ::backtrace(trace, 50);
	for (int i = res; i < 50; i++)
		trace[i] = nullptr;
}

MemoryChecker::snapshot MemoryChecker::calculate_difference() const noexcept {
	snapshot res;
	res.num_malloc = g_global.num_malloc - m_start.num_malloc;
	res.num_malloc_failed = g_global.num_malloc_failed - m_start.num_malloc_failed;
	res.num_free = g_global.num_free - m_start.num_free;
	res.num_realloc = g_global.num_realloc - m_start.num_realloc;
	res.num_realloc_failed = g_global.num_realloc_failed - m_start.num_realloc_failed;
	res.num_realloc_moved = g_global.num_realloc_moved - m_start.num_realloc_moved;
	res.num_memalign = g_global.num_memalign - m_start.num_memalign;
	res.num_memalign_failed = g_global.num_memalign_failed - m_start.num_memalign_failed;
	res.num_chunks_allocated = g_global.num_chunks_allocated - m_start.num_chunks_allocated;
	res.num_chunks_allocated_max = g_global.num_chunks_allocated_max - m_start.num_chunks_allocated_max;
	res.num_bytes_allocated = g_global.num_bytes_allocated - m_start.num_bytes_allocated;
	res.num_bytes_allocated_max = g_global.num_bytes_allocated_max - m_start.num_bytes_allocated_max;
	res.bt_max_chunks = g_global.bt_max_chunks;
	res.bt_max_bytes = g_global.bt_max_bytes;
	return res;
}

extern "C" void* __libc_malloc(size_t);
extern "C" void* __libc_realloc(void*, size_t);
extern "C" void __libc_free(void*);
extern "C" void* __libc_memalign(size_t alignment, size_t size);

static thread_local bool in_memchecker = false;
void* MemoryChecker::mc_malloc(size_t size, const void* caller) {
	if (in_memchecker) return __libc_malloc(size);
	in_memchecker = true;
	void* ptr = __libc_malloc(size);

	g_global.num_malloc++;
	if (ptr == nullptr) {
		g_global.num_malloc_failed++;
		in_memchecker = false;
		return ptr;
	}

	size = malloc_usable_size(ptr);

	g_global.num_bytes_allocated += size;
	g_global.num_chunks_allocated++;
	if (g_global.num_chunks_allocated > g_global.num_chunks_allocated_max) {
		g_global.num_chunks_allocated_max = g_global.num_chunks_allocated;
		g_global.bt_max_chunks.capture();
	}
	if (g_global.num_bytes_allocated > g_global.num_bytes_allocated_max) {
		g_global.num_bytes_allocated_max = g_global.num_bytes_allocated;
		g_global.bt_max_bytes.capture();
	}
	in_memchecker = false;
	return ptr;
}

void* MemoryChecker::mc_realloc(void* cptr, size_t size, const void* caller) {
	if(in_memchecker) return __libc_realloc(cptr, size);
	in_memchecker = true;
	auto oldsize = malloc_usable_size(cptr);
	void* ptr = __libc_realloc(cptr, size);

	g_global.num_realloc++;
	if (ptr == nullptr) {
		g_global.num_realloc_failed++;
		in_memchecker = false;
		return ptr;
	}
	if (cptr == ptr) {
		g_global.num_realloc_moved++;
	}

	g_global.num_bytes_allocated += (static_cast<ssize_t>(size) - static_cast<ssize_t>(oldsize));
	if (g_global.num_chunks_allocated > g_global.num_chunks_allocated_max) {
		g_global.num_chunks_allocated_max = g_global.num_chunks_allocated;
		g_global.bt_max_chunks.capture();
	}
	if (g_global.num_bytes_allocated > g_global.num_bytes_allocated_max) {
		g_global.num_bytes_allocated_max = g_global.num_bytes_allocated;
		g_global.bt_max_bytes.capture();
	}
	in_memchecker = false;
	return ptr;
}

void MemoryChecker::mc_free(void* ptr, const void* caller) {
	if(in_memchecker) return __libc_free(ptr);
	if (ptr == nullptr) return;
	in_memchecker = true;
	auto oldsize = malloc_usable_size(ptr);

	__libc_free(ptr);

	g_global.num_free++;
	g_global.num_bytes_allocated -= oldsize;
	g_global.num_chunks_allocated--;
	in_memchecker = false;
}

void* MemoryChecker::mc_memalign(size_t alignment, size_t size, const void* caller) {
	void* ptr = __libc_memalign(alignment, size);
	if(in_memchecker) return ptr;
	in_memchecker = true;

	g_global.num_memalign++;
	if (ptr == nullptr) {
		g_global.num_memalign_failed++;
		in_memchecker = false;
		return ptr;
	}

	size = malloc_usable_size(ptr);

	g_global.num_bytes_allocated += size;
	g_global.num_chunks_allocated++;
	if (g_global.num_chunks_allocated > g_global.num_chunks_allocated_max) {
		g_global.num_chunks_allocated_max = g_global.num_chunks_allocated;
		g_global.bt_max_chunks.capture();
	}
	if (g_global.num_bytes_allocated > g_global.num_bytes_allocated_max) {
		g_global.num_bytes_allocated_max = g_global.num_bytes_allocated;
		g_global.bt_max_bytes.capture();
	}
	in_memchecker = false;
	return ptr;
}


std::ostream& operator<<(std::ostream& str, const MemoryChecker::stacktrace& o) {
	int n = 0;
	for(; n < 49; n++) if(o.trace[n+1] == nullptr) break;
	auto strings = backtrace_symbols(o.trace, n);
	if (strings == NULL) {
		str << "<backtrace_symbols failed>";
		return str;
	}
	for (int j = 0; j < n; j++)
		str << strings[j] << "\n";
	free(strings);
	return str;
}

std::ostream& operator<<(std::ostream& str, const MemoryChecker& o) {
	auto diff = o.calculate_difference();
	str << "==== Memory report ====\n";
	str << "num_malloc =               " << diff.num_malloc << "\n";
	str << "num_malloc_failed =        " << diff.num_malloc_failed << "\n";
	str << "num_free =                 " << diff.num_free << "\n";
	str << "num_realloc =              " << diff.num_realloc << "\n";
	str << "num_realloc_failed =       " << diff.num_realloc_failed << "\n";
	str << "num_realloc_moved =        " << diff.num_realloc_moved << "\n";
	str << "num_memalign =             " << diff.num_memalign << "\n";
	str << "num_memalign_failed =      " << diff.num_memalign_failed << "\n";
	str << "num_chunks_allocated =     " << diff.num_chunks_allocated << "\n";
	str << "num_chunks_allocated_max = " << diff.num_chunks_allocated_max << "\n";
	str << "num_bytes_allocated =      " << diff.num_bytes_allocated << "\n";
	str << "num_bytes_allocated_max =  " << diff.num_bytes_allocated_max << "\n";
	str << "max_chunks_at:\n" << diff.bt_max_chunks;
	str << "max_bytes_at:\n" << diff.bt_max_bytes;
	return str;
}


extern "C" void* malloc(size_t size) {
	return MemoryChecker::mc_malloc(size, __builtin_return_address(0));
}
extern "C" void* realloc(void* ptr, size_t size) {
	return MemoryChecker::mc_realloc(ptr, size, __builtin_return_address(0));
}
extern "C" void* memalign(size_t alignment, size_t size) {
	return MemoryChecker::mc_memalign(alignment, size, __builtin_return_address(0));
}
extern "C" void free(void* ptr) {
	MemoryChecker::mc_free(ptr, __builtin_return_address(0));
}
extern "C" int posix_memalign(void** memptr, size_t alignment, size_t size) {
	*memptr = memalign(alignment, size);
	if (!*memptr) return ENOMEM;
	return 0;
}
