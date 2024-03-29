/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-29 12:39:59 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-03-29 12:55:57
 */

#pragma once
#include "dsm/type.h"

#include <vector>

namespace dsm {

struct Object {
  int id;
  const char *name;

  // `data` is stored in local memory in `std::vector`
  // with STL's allocator. It does not care about what 
  // to be stored in the memory. All represented as `u_int8_t`.
  std::vector<Byte> data;
}; // struct Object


} // namespace dsm
