/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-04-01 11:51:52 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-04-01 17:02:28
 */

#pragma once

#include "dsm/object.h"
#include "dsm/type.h"

#include <array>

namespace dsm {
namespace types {

class Int final: public Object  {

 public: 
  Int() = delete;

  // users must construct this first
  explicit Int(Object &&rhs) : Object(std::move(rhs)) {}

  ~Int() override = default;

  Int(const Int &) = delete;
  Int &operator=(const Int &) = delete;
  Int(Int &&) = delete;
  Int &operator=(Int &&) = delete;

  // implicitly write
  Int &operator=(int i) {
    std::array<Byte, sizeof(int)> tmp{0, 0, 0, 0};
    const Byte *byte_ptr = reinterpret_cast<const Byte *>(&i);
    for (size_t i = 0; i < tmp.size(); i++) {
      tmp[i] = byte_ptr[i];
    }
    write(0, tmp);
    return *this;
  }

  // implicitly read
  operator int() {
    auto tmp = read(0, sizeof(int));
    int i = 0;
    Byte *byte_ptr = reinterpret_cast<Byte *>(&i);
    for (size_t i = 0; i < tmp.size(); i++) {
      byte_ptr[i] = tmp[i];
    }
    return i;
  }
};

} // namespace types
} // namespace dsm