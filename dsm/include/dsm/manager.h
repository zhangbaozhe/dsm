/*
 * @Brief: Following the POSIX `shm_*` and `mmap` to build the DSM framework
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-29 12:56:32 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-04-01 21:40:46
 */
#pragma once

#include "dsm/type.h"
#include "dsm/object.h"
#include "dsm/config.h"


#include <unordered_map>
#include <memory>
#include <thread>

#define CPPHTTPLIB_ZLIB_SUPPORT
#include <httplib.h>
#include <gsl-lite.hpp>


namespace dsm {

class Manager {
 public: 
  explicit Manager(const char *config_path);
  ~Manager();

  Manager() = delete;
  Manager(const Manager &) = delete;
  Manager(Manager &&) = delete;
  Manager &operator=(const Manager &) = delete;
  Manager &operator=(Manager &&) = delete;


  Object* mmap(const std::string &name, size_t length);

  template <typename T>
  T *mmap(const std::string &name) {
     static_assert(std::is_same<T, Int32>::value || 
                  std::is_same<T, UInt32>::value || 
                  std::is_same<T, Int64>::value || 
                  std::is_same<T, UInt64>::value || 
                  std::is_same<T, Float>::value || 
                  std::is_same<T, Double>::value || 
                  std::is_same<T, Char>::value || 
                  std::is_same<T, Bool>::value, 
                  "T must be an Element type");
    return static_cast<T *>(mmap(name, sizeof(typename T::value_type)));
  }
  
  int munmap(const std::string &name);

  std::vector<Object *> find_objects(const std::string &name_prefix);


 private: 
  std::unordered_map<std::string, Object *> m_database;
  Config m_config;

  friend Object;
  gsl::span<Byte> read(const std::string &name, size_t offset, size_t length);
  void write(const std::string &name, size_t offset, gsl::span<Byte> data);

  std::vector<std::unique_ptr<httplib::Client>> m_clients;
  std::unique_ptr<httplib::Server> m_server;

  std::thread m_listen_thread;



}; // class Manager

inline std::string random_string( size_t length )
{
  auto randchar = []() -> char
  {
    const char charset[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[ rand() % max_index ];
  };
  std::string str(length,0);
  std::generate_n( str.begin(), length, randchar );
  return str;
}

template <typename T>
inline std::vector<Object *> generate_objects(Manager &manager, size_t num_objects, const std::string &prefix = "") {
  static_assert(std::is_same<T, Int32>::value || 
                std::is_same<T, UInt32>::value || 
                std::is_same<T, Int64>::value || 
                std::is_same<T, UInt64>::value || 
                std::is_same<T, Float>::value || 
                std::is_same<T, Double>::value || 
                std::is_same<T, Char>::value || 
                std::is_same<T, Bool>::value, 
                "T must be an Element type");
  std::vector<Object *> objects;
  for (size_t i = 0; i < num_objects; i++) {
    objects.push_back(manager.mmap(prefix + "_" + random_string(8) + "_" + std::to_string(i), sizeof(typename T::value_type)));
  }
  return objects;
}

} // namespace dsm
