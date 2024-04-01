# design

This distributed shared memory framework is designed to be close to 
the `shm_` and `mmap` shared memory functions in the POSIX standard. 

The provided `dsm` framework offers one single `dsm::Manager` class 
for C++ user to construct the DSM. Although similar to the shared memory
pattern in POSIX, we provide a more modern memory representation to the 
user, an `dsm::Object` class. Under the hood of this class, the memory 
is managed by the STL's allocator using `std::vector<Byte>`. 

In this framework, user does not care about page size or others in the low 
level. The purpose for the given `dsm::Object` is to provide an object-based sharing mechanism. Users who wish to use their own designed object, say `Matrix3x3`, should explicitly make their own object inherit from the `dsm::Object` and override some indexing operators.  