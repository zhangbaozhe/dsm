#include "dsm/manager.h"

int main(int argc, char *argv[]) {
  dsm::Manager manager(argv[1]);

  dsm::Int32 *data = manager.mmap<dsm::Int32>("test");
  for (size_t i = 0; i < 1000; i++) {
    *data += 1;
  }

  sleep(1);
  std::cout << "Result " << *data << std::endl;
}