/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-30 16:13:55 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-03-30 18:58:05
 */

#include "dsm/object.h"
#include "dsm/manager.h"

namespace dsm {

std::vector<Byte> Object::read(size_t offset, size_t length) {
  if (offset + length > m_data.size()) {
    throw std::out_of_range("Read out of range");
  }
  return m_manager_ptr->read(m_name, offset, length);
}

void Object::write(size_t offset, const std::vector<Byte> &data) {
  if (offset + data.size() > m_data.size()) {
    throw std::out_of_range("Write out of range");
  }
  m_manager_ptr->write(m_name, offset, data);
}

} // namespace dsm