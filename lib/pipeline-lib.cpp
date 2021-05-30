#include <map>
#include <pipeline-itf.h>
#include <pipeline-lib.h>
#include <snowboy-error.h>
#include <snowboy-io.h>
#include <snowboy-options.h>
#include <snowboy-utils.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace snowboy {
	void PipelineItf::SetResource(const std::string& param_1) {
		if (m_isInitialized)
			throw snowboy_exception{"class has already been initialized, you have to call SetResource before calling Init()"};
		ParseOptions opts{""};
		std::string opts_out;
		UnpackPipelineResource(param_1, &opts_out);
		FilterConfigString(false, "--" + this->OptionPrefix(), &opts_out);
		this->RegisterOptions(this->OptionPrefix(), &opts);
		opts.ReadConfigString(opts_out);
	}

	PipelineItf::~PipelineItf() {}

	void PackPipelineResource(const std::string& a, const std::string& b) {
		PackPipelineResource(true, a, b);
	}

	void PackPipelineResource(bool binary, const std::string& filename, const std::string& options) {
		std::map<std::string, int> filename_map;
		std::vector<std::string> filenames;

		std::vector<std::string> config_parts;
		SplitStringToVector(options, global_snowboy_whitespace_set, &config_parts);
		for (auto& opt : config_parts) {
			std::vector<std::string> opt_parts;
			SplitStringToVector(opt, "=", &opt_parts);
			if (opt_parts.size() == 1) continue;
			if (opt_parts.size() == 2) {
				auto pos = opt_parts[0].find("filename", 0);
				if (pos != std::string::npos) {
					auto id = filenames.size();
					filename_map[opt_parts[1]] = id;
					filenames.push_back(opt_parts[1]);
					opt = opt_parts[0] + "=" + std::to_string(id);
				}
			} else
				throw snowboy_exception{"Bad option in configuration string: \"" + opt
										+ "\"; Supported format is --option=value, or --option for boolean types"};
		}
		Output out{filename, binary};
		auto stream = out.Stream();
		WriteToken(binary, "<ResourceFileOffsets>", stream);
		WriteToken(binary, "<NumOffsets>", stream);
		WriteBasicType<int32_t>(binary, filenames.size(), stream);
		size_t offset = 0;
		for (auto& e : filenames) {
			WriteBasicType<int32_t>(binary, offset, stream);
			struct stat stat_buf;
			int rc = stat(e.c_str(), &stat_buf);
			if (rc != 0)
				throw snowboy_exception{"Fail to open resource file \"" + e + "\""};
			offset += stat_buf.st_size;
		}
		WriteToken(binary, "</ResourceFileOffsets>", stream);
		WriteToken(binary, "<Configuration>", stream);
		WriteToken(binary, "<NumConfigs>", stream);
		WriteBasicType<int32_t>(binary, config_parts.size(), stream);
		for (const auto& e : config_parts)
			WriteToken(binary, e, stream);
		WriteToken(binary, "</Configuration>", stream);
		for (const auto& e : filenames) {
			std::ifstream file{e, std::ios::binary};
			if (!file.is_open())
				throw snowboy_exception{"Fail to open resource file \"" + e + "\""};
			*stream << file.rdbuf();
		}
	}

	void UnpackPipelineResource(const std::string& filename, std::string* options_out) {
		Input in{filename};
		auto stream = in.Stream();
		bool binary = in.is_binary();
		ExpectToken(binary, "<ResourceFileOffsets>", stream);
		ExpectToken(binary, "<NumOffsets>", stream);
		int num_offsets = 0;
		ReadBasicType<int32_t>(binary, &num_offsets, stream);
		std::vector<int> offsets(num_offsets);
		for (auto& e : offsets) {
			ReadBasicType<int32_t>(binary, &e, stream);
		}
		ExpectToken(binary, "</ResourceFileOffsets>", stream);
		ExpectToken(binary, "<Configuration>", stream);
		ExpectToken(binary, "<NumConfigs>", stream);
		int num_configs = 0;
		ReadBasicType<int32_t>(binary, &num_configs, stream);
		std::vector<std::string> configs(num_configs);
		for (auto& e : configs) {
			ReadToken(binary, &e, stream);
		}
		ExpectToken(binary, "</Configuration>", stream);
		size_t res_start = stream->tellg(); // TODO: This is not portable (but works on linux & windows for binary files)
		size_t reserve_size = 0;
		for (auto& opt : configs) {
			std::vector<std::string> opt_parts;
			SplitStringToVector(opt, "=", &opt_parts);
			if (opt_parts.size() == 1) {
			} else if (opt_parts.size() == 2) {
				auto pos = opt_parts[0].find("filename", 0);
				if (pos != std::string::npos) {
					auto idx = std::stoul(opt_parts[1]);
					if (idx >= offsets.size()) {
						throw snowboy_exception{"Bad config file, index " + std::to_string(idx)
												+ " exceeds number of offsets (" + std::to_string(offsets.size()) + ")"};
					}
					opt = opt_parts[0] + "=" + filename + global_snowboy_offset_delimiter + std::to_string(offsets[idx] + res_start);
				}
			} else {
				throw snowboy_exception{"Bad option in configuration string: \"" + opt
										+ "\"; Supported format is --option=value, or --option for boolean types"};
				return;
			}
			reserve_size += opt.size();
		}
		options_out->resize(0);
		// All config entries + (n-1) spaces
		options_out->reserve(reserve_size + configs.size() - 1);
		for (size_t i = 0; i < configs.size(); i++) {
			if (i != 0) options_out->append(" ");
			options_out->append(configs[i]);
		}
	}
} // namespace snowboy
