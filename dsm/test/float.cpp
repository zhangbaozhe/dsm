#include "dsm/manager.h"

int main(int argc, char *argv[]) {
  dsm::Manager manager(argv[1]);

  dsm::Float *data = manager.mmap<dsm::Float>("test");
  for (size_t i = 0; i < 1000; i++) {
    *data += 1.0;
  }

  sleep(1);
  std::cout << "Result " << *data << std::endl;
}