/*
 * @Brief: Following the POSIX `shm_*` and `mmap` to build the DSM framework
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-29 12:56:32 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-04-18 14:18:19
 */
#pragma once

#include "dsm/type.h"
#include "dsm/object.h"
#include "dsm/config.h"
#include "dsm/utils/safe_map.h"


#include <unordered_map>
#include <memory>
#include <thread>
#include <atomic>

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


  void create_mutex(const std::string &name);
  void delete_mutex(const std::string &name);

  /**
   * @brief Tell other peers that this process is processing the object, 
   *       and other peers should wait until the process is done.
   * 
   * @param name 
   */
  void mutex_lock(const std::string &name);
  void mutex_unlock(const std::string &name);

  /**
   * @brief Create a new object with the given name and size
   * 
   * @param name 
   * @param length 
   * @return Object* 
   */
  Object* mmap(const std::string &name, size_t length);

  /**
   * @brief Create a new object with the given name and size
   * 
   * @tparam T 
   * @param name 
   * @return T* 
   */
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
  
  /**
   * @brief Delete the object with the given name
   * 
   * @param name 
   * @return void 
   */
  void munmap(const std::string &name);

  /**
   * @brief Find the object with the given prefix
   * 
   * @param name_prefix 
   * @return std::vector<Object *> 
   */
  std::vector<Object *> find_objects(const std::string &name_prefix);


 private: 
  utils::SafeMap<std::string, Object *> m_database;

  /**
   * NOTE: If we want to achieve easy mutex implementation in this DSM framework,
   *       the underlying mutex `int` should be implemented with `strong` atomicity, 
   *       not in the local memory order, but across the whole DSM framework.
   * GOAL: Need to implement the `strong` atomicity for the `int` type in the `SafeMap` class.
   */
  utils::SafeMap<std::string, std::atomic_int> m_mutexes;
  int m_pattern;

  Config m_config;

  friend Object;
  gsl::span<Byte> read(const std::string &name, size_t offset, size_t length);
  void write(const std::string &name, size_t offset, gsl::span<Byte> data);

  std::vector<std::unique_ptr<httplib::Client>> m_clients;
  std::unique_ptr<httplib::Server> m_server;
  std::unique_ptr<httplib::Client> m_param_server;

  std::thread m_listen_thread;
  std::mutex m_internal_thread_mutex;

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
