/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-30 20:46:11 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-03-30 23:05:54
 */

#include "dsm/manager.h"

int main(int argc, char *argv[]) {
  dsm::Manager manager(argv[1]);

  std::cout << "test" << std::endl;
  auto obj = manager.mmap("test", 1024);

  std::cout << "test" << std::endl;
  manager.mmap("test1", 1024);

  std::cout << "test" << std::endl;
  manager.mmap("test2", 1024);

  std::cout << "test" << std::endl;
  manager.mmap("test3", 1024);

  manager.munmap("test1");
  manager.munmap("test2");
  return 0;
}