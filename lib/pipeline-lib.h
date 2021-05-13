#pragma once
#include <string>

namespace snowboy {
	void PackPipelineResource(const std::string&, const std::string&);
	void PackPipelineResource(bool, const std::string&, const std::string&);
	void UnpackPipelineResource(const std::string&, std::string*);
} // namespace snowboy