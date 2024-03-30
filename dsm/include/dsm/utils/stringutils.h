/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-29 15:43:11 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-03-29 16:14:51
 */

#pragma once

#include <string>
#include <vector>

namespace dsm {
namespace utils {

std::string &ltrim(std::string &s);

std::string &rtrim(std::string &s);

std::string &trim(std::string &s);

std::string read_file(const char *filename);

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);

std::string string_to_hex(const std::string &input);

bool startswith(const std::string &s, const std::string &prefix);
bool endswith(const std::string &s, const std::string &suffix);

uint64_t parse_internet_address(const std::string &ip);

std::string unparse_internet_address(uint64_t ip);


} // namespace utils
} // namespace dsm