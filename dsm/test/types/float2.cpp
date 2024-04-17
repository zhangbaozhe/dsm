#include "dsm/manager.h"

int main(int argc, char *argv[]) {
  dsm::Manager manager(argv[1]);

  dsm::Float *data = manager.mmap<dsm::Float>("test");
  // *data = 0;

  for (size_t i = 0; i < 100; i++) {
    *data += 1;
    std::cout << (float)*data << std::endl;
  }
  
  sleep(1);
  std::cout << *data << std::endl;
}