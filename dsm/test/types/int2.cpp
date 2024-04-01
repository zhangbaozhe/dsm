#include "dsm/manager.h"
#include "dsm/types/int.h"

int main(int argc, char *argv[]) {
  dsm::Manager manager(argv[1]);

  auto obj = manager.mmap("test", sizeof(int), dsm::MapProt::READ, dsm::MapFlag::SHARED);


  dsm::types::Int *i = (dsm::types::Int *)obj;
  *i = 42;
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