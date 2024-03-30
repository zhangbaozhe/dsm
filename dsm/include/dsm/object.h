/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-29 12:39:59 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-03-30 22:57:05
 */

#pragma once
#include "dsm/type.h"

#include <vector>

namespace dsm {
namespace consensus {
class ServerWorker;
} // namespace consensus

class Manager;

/**
 * @brief Object may be served as a parent class 
 * 
 */
class Object {
 public:
  Object() : m_name(nullptr), m_manager_ptr(nullptr) {}
  Object(std::string name, Manager *manager_ptr) : 
    m_name(name), m_manager_ptr(manager_ptr) {}
  virtual ~Object() = default;
  
  std::string get_name() const { return m_name; };

  std::vector<Byte> read(size_t offset, size_t length);

  void write(size_t offset, const std::vector<Byte> &data);

 private: 
  std::string m_name;
  // `data` is stored in local memory in `std::vector`
  // with STL's allocator. It does not care about what 
  // to be stored in the memory. All represented as `u_int8_t`.
  // TODO: serialization & deserialization needed
  std::vector<Byte> m_data;

  friend Manager; 
  friend consensus::ServerWorker;
  // Object's read & write operations are notified to `Manager`
  Manager *m_manager_ptr;
}; // struct Object


} // namespace dsm
