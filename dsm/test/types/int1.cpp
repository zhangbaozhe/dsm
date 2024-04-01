#include "dsm/manager.h"

int main(int argc, char *argv[]) {
  dsm::Manager manager(argv[1]);
  sleep(5);

  dsm::Int32 *data = (dsm::Int32 *)manager.mmap("test", sizeof(int), dsm::MapProt::NONE, dsm::MapFlag::SHARED);


  for (auto i = 0; i < 10; i++) {
    std::cout << *data << std::endl;
    sleep(2);
  }
}