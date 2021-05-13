#pragma once
#include <string>
#include <vector>
#include <gtest/gtest.h>

template<typename... Args>
void GTEST_WARN(const std::string& fmt, Args... args) {
    std::string msg = "[          ] " + fmt + "\n";
    testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, msg.c_str(), std::forward<Args>(args)...);
}

std::vector<short> read_sample_file(const std::string& filename);
bool file_exists(const std::string& name);
std::string detect_project_root();