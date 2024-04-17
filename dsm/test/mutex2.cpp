#include "dsm/manager.h"

int main(int argc, char *argv[]) {
  dsm::Manager manager(argv[1]);
  manager.create_mutex("test");

  manager.mutex_lock("test");
  dsm::Int32 *data = manager.mmap<dsm::Int32>("test");
  manager.mutex_unlock("test");
  // *data = 0;
  // manager.mutex_lock("test");
  // std::this_thread::sleep_for(std::chrono::seconds(1));
  // manager.mutex_unlock("test");

  for (int i = 0; i < 2000; i++) {
    manager.mutex_lock("test");
    *data += 1;
    std::cout << *data << std::endl;
    manager.mutex_unlock("test");
  }

  std::cout << "Done" << std::endl;
  std::cout << "Result" << *data << std::endl;
}