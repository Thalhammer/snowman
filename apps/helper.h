#pragma once
#include <string>
#include <vector>

std::vector<short> read_sample_file(const std::string& filename, bool treat_wave = false);
bool file_exists(const std::string& name);
std::string detect_project_root();
std::string read_file(const std::string& file);