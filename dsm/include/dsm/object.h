/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-29 12:39:59 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-04-01 16:52:44
 */

#pragma once
#include "dsm/type.h"

#include <vector>
#include <gsl-lite.hpp>

namespace dsm {

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
  

  // Explicitly disable copy constructor and copy assignment operator
  // to avoid implicitly registering the object in `Manager`.
  // Users should use `Manager::mmap` to create an object
  Object(const Object &) = delete;
  Object &operator=(const Object &) = delete;

  // However uses can transfer the ownership of the object
  // this does not affect the underlying manager's database
  Object(Object &&);
  Object &operator=(Object &&) noexcept;

  virtual ~Object() = default;
  
  std::string get_name() const { return m_name; };

 protected: 
  gsl::span<Byte> read(size_t offset, size_t length);

  void write(size_t offset, gsl::span<Byte> data);

 private: 
  std::string m_name;
  // `data` is stored in local memory in `std::vector`
  // with STL's allocator. It does not care about what 
  // to be stored in the memory. All represented as `u_int8_t`.
  // TODO: serialization & deserialization needed
  std::vector<Byte> m_data;

  friend Manager; 
  // Object's read & write operations are notified to `Manager`
  Manager *m_manager_ptr;
}; // class Object




} // namespace dsm
