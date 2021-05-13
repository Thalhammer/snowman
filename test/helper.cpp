#include <helper.h>
#include <stdexcept>
#include <sys/stat.h>

std::vector<short> read_sample_file(const std::string& filename) {

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
		if (fread(&wavhdr->datasize, 4, 1, fi) != 4) throw std::runtime_error("fread failed");
	};

	bool isWav = (filename.size() > 4 && filename.rfind(".wav") != filename.size() - 4);

	FILE* f = fopen(filename.c_str(), "rb");
	if (f == NULL)
	{
		perror("Error opening file");
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