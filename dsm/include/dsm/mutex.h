/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-04-17 13:45:25 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-04-17 13:55:34
 */

#include <string>

namespace dsm {

class Manager;

class Mutex {
 public: 
  Mutex() : m_manager_ptr(nullptr), m_name("") {};
  Mutex(Manager *manager_ptr, const std::string &name) : m_manager_ptr(manager_ptr), m_name(name) {};
  ~Mutex() = default;

  Mutex(const Mutex &) = delete;
  Mutex(Mutex &&) = delete;
  Mutex &operator=(const Mutex &) = delete;
  Mutex &operator=(Mutex &&) = delete;

  void lock();
  void unlock();

  bool try_lock();
  std::string get_name() const { return m_name; };

 private: 
  Manager *m_manager_ptr;
  std::string m_name;

}; // class Mutex


} // namespace dsm