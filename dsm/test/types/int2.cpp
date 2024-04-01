#include "dsm/manager.h"

int main(int argc, char *argv[]) {
  dsm::Manager manager(argv[1]);

  auto obj = manager.mmap("test", sizeof(int), dsm::MapProt::READ, dsm::MapFlag::SHARED);

  auto ptrs = dsm::generate_objects<int>(manager, 10);
  // TODO:  how to let the other peer know this object ?
  dsm::Array<dsm::Int32, 10> array(ptrs);

  dsm::Int32 *i = (dsm::Int32 *)obj;
  *i = 42;

  for (size_t j = 0; j < 10; j++) {
    array[j] = j;
    std::cout << (int)array[j] << std::endl;
    sleep(1);
  }
  // for (int j = 0; j < 10; j++) {
  //   // (*i) = (*i) + 1; 
  //   std::cout << (int)*i << std::endl;
  //   sleep(1);
  // }
  // sleep(100);
  // destruction here will modify the manager
  // this thread here starts to destruct the manager
  // shared memory aka `m_database` is to be cleared
  return 0;
}