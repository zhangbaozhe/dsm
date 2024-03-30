/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-29 12:53:00 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-03-30 20:48:18
 */

#pragma once

#include <json.hpp>

namespace dsm {

using Byte = unsigned char;

enum class OpenFlag : int {
  RDONLY, 
  RDWR, 
  CREATE, 
  EXCL, 
  TRUNC, 
}; // enum class OFlag

enum class MapProt {
  EXEC, 
  READ, 
  WRITE, 
  NONE, 
}; // enum class MapProt

enum class MapFlag {
  SHARED, 
  PRIVATE, 
}; // enum class MapFlat

using Json = nlohmann::json;

} // namespace dsm