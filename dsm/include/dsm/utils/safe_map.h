/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-04-17 18:23:32 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-04-17 18:31:43
 */

#pragma once

#include <unordered_map>
#include <mutex>

namespace dsm {
namespace utils {

template <typename Key, typename Value>
class SafeMap {
 public:
  SafeMap() = default;
  
  void insert(const Key& key, const Value& value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_map[key] = value;
  }

  bool find(const Key& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_map.find(key) != m_map.end();
  }

  bool erase(const Key& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_map.erase(key);
  }

  bool empty() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_map.empty();
  }

  size_t size() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_map.size();
  }

  void clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_map.clear();
  }

  std::unordered_map<Key, Value> get_map() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_map;
  }

  Value& operator[](const Key& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_map[key];
  }


 private: 
  std::unordered_map<Key, Value> m_map;
  std::mutex m_mutex;
}; // class SafeMap


} // namespace utils
} // namespace dsm