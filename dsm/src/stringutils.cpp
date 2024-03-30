/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-29 15:51:19 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-03-29 16:15:14
 */

#include "dsm/utils/stringutils.h"

#include <arpa/inet.h>

#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <vector>

namespace dsm {
namespace utils {

std::string &ltrim(std::string &s) {
  s.erase(
    s.begin(), 
    std::find_if(
      s.begin(), 
      s.end(), 
      std::not1(std::ptr_fun<int, int>(std::isspace))
    )
  );
  return s;
}

std::string &rtrim(std::string &s) {
  s.erase(
    std::find_if(
      s.rbegin(), 
      s.rend(), 
      std::not1(std::ptr_fun<int, int>(std::isspace))
    ).base(), 
    s.end()
  );
  return s;
}

std::string &trim(std::string &s) {
  return ltrim(rtrim(s));
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

std::string read_file(const char *filename) {
  std::ifstream t(filename, std::ifstream::in);
  std::string s((std::istreambuf_iterator<char>(t)), 
      std::istreambuf_iterator<char>());
  return s;
}

std::string string_to_hex(const std::string &input) {
  static const char *const lut = "0123456789ABCDEF";
  size_t len = input.length();
  std::string output;
  output.reserve(2 * len);
  for (size_t i = 0; i < len; ++i) {
    const unsigned char c = input[i];
    output.push_back(lut[c >> 4]);
    output.push_back(lut[c & 15]);
  }
  return output;
}

bool startswith(const std::string &s, const std::string &prefix) {
  return prefix.length() <= s.length() 
    && std::equal(prefix.begin(), prefix.end(), s.begin());
}

bool endswith(const std::string &s, const std::string &suffix) {
  return suffix.length() <= s.length()
    && std::equal(suffix.begin(), suffix.end(), s.end() - suffix.size());
}

uint64_t parse_internet_address(const std::string &ip) {
  uint64_t ip_addr = 0;
  uint8_t buf[4];
  int r = inet_pton(AF_INET, ip.c_str(), buf);
  if (r <= 0) {
    return 0;
  }
  for (int i = 0; i < 4; i++) {
    ip_addr <<= 8;
    ip_addr |= buf[i] & 0xff;
  }
  return ip_addr;
}

std::string unparse_internet_address(uint64_t ip) {
  uint32_t half_of_long = static_cast<uint32_t>(htonl(ip));
  struct in_addr ip_addr;
  ip_addr.s_addr = half_of_long;
  return inet_ntoa(ip_addr);
}


} // namespace utils
} // namespace dsm