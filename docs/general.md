- How to determine local & shared memory?
  Shared memory allocation needs to be explicitly called with `dsm::malloc` or `dsm_malloc`. 
  Shared memory can be accessed by different process (on different clusters) by using 
  `dsm::shm_` or `dsm_shm_`. These APIs are similar to those in POSIX standard.