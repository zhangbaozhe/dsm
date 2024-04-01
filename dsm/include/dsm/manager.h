/*
 * @Brief: Following the POSIX `shm_*` and `mmap` to build the DSM framework
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-29 12:56:32 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-04-01 17:34:33
 */
#pragma once

#include "dsm/type.h"
#include "dsm/object.h"
#include "dsm/config.h"


#include <unordered_map>
#include <memory>
#include <thread>

// #define CPPHTTPLIB_ZLIB_SUPPORT
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


  Object* mmap(const std::string &name, size_t length, MapProt prot, MapFlag flags);
  
  int munmap(const std::string &name);

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


} // namespace dsm
