/*
 * @Brief: Object implementation
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-30 16:13:55 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-04-01 17:24:13
 */

#include "dsm/object.h"
#include "dsm/manager.h"

namespace dsm {

Object::Object(Object &&other) {
  m_name = std::move(other.m_name);
  m_manager_ptr = other.m_manager_ptr;
  m_data = std::move(other.m_data);
  other.m_manager_ptr = nullptr;

  m_manager_ptr->m_database[m_name] = this;
}

Object &Object::operator=(Object &&other) noexcept{
  if (this != &other) {
    m_name = std::move(other.m_name);
    m_manager_ptr = other.m_manager_ptr;
    m_data = std::move(other.m_data);
    other.m_manager_ptr = nullptr;

    m_manager_ptr->m_database[m_name] = this;
  }
  return *this;
}

gsl::span<Byte> Object::read(size_t offset, size_t length) const {
  if (offset + length > m_data.size()) {
    throw std::out_of_range("Read out of range");
  }
  return m_manager_ptr->read(m_name, offset, length);
}

void Object::write(size_t offset, gsl::span<Byte> data) {
  if (offset + data.size() > m_data.size()) {
    throw std::out_of_range("Write out of range");
  }
  m_manager_ptr->write(m_name, offset, data);
}

} // namespace dsm