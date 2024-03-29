/*
 * @Brief: Following the POSIX `shm_*` and `mmap` to build the DSM framework
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-29 12:56:32 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-03-29 13:22:56
 */
#pragma once

#include "dsm/type.h"
#include "dsm/object.h"

#include <unordered_map>

namespace dsm {

class Manager {
 public: 
  Manager(const char *config_path);

  Manager() = delete;
  Manager(const Manager &) = delete;
  Manager(Manager &&) = delete;
  Manager &operator=(const Manager &) = delete;
  Manager &operator=(Manager &&) = delete;

  int open(const char *name, OpenFlag flag);

  int unlink(const char *name);

  Object* mmap(const char *name, size_t length, MapProt prot, MapFlag flags);
  
  int munmap(const char *name);

 private: 
  std::unordered_map<const char *, Object *> m_database;


}; // class Manager


} // namespace dsm
