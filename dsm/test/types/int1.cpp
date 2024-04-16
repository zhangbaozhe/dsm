#include "dsm/manager.h"

int main(int argc, char *argv[]) {
  dsm::Manager manager(argv[1]);

  // dsm::Int32 *data = (dsm::Int32 *)manager.mmap("test", sizeof(int));

  // auto data2 = manager.mmap<dsm::Int32>("test2");

  // for (auto i = 0; i < 10; i++) {
  //   std::cout << *data << std::endl;
  //   sleep(2);
  // }

  // auto ptrs = manager.find_objects("array");
  // dsm::Array<dsm::Int32, 10> array(ptrs);

  // for (size_t i = 0; i < 10; i++) {
  //   std::cout << (int)array[i] << std::endl;
  //   sleep(1);
  // }

  // auto ptrs = manager.find_objects("matrix");
  // dsm::Matrix<dsm::Int32, 4, 4> matrix(ptrs);
  // sleep(1);
  dsm::Int32 *data = manager.mmap<dsm::Int32>("test");
  // *data = 0;
  // TODO: sync
  for (size_t i = 0; i < 100; i++) {
    *data += 1;
    std::cout << *data << std::endl;
  }
}