#include "dsm/manager.h"
#include "spdlog/spdlog.h"

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

  auto start_time = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 1000; i++) {
    manager.mutex_lock("test"); // FIXME: potentially call at the same time?
    *data += 1;
    std::cout << *data << std::endl;
    manager.mutex_unlock("test");
  }
  auto end_time = std::chrono::high_resolution_clock::now();
  spdlog::info("Time: {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count());

  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Done" << std::endl;
  std::cout << "Result" << *data << std::endl;
}