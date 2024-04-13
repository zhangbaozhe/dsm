add_executable(allocation1
  allocation1.cpp
)
add_executable(allocation2
  allocation2.cpp
)

add_executable(int1
  types/int1.cpp
)
add_executable(int2
  types/int2.cpp
)
add_executable(float1
  types/float1.cpp
)
add_executable(float2
  types/float2.cpp
)

target_link_libraries(allocation1 dsm)
target_link_libraries(allocation2 dsm)

target_link_libraries(int1 dsm)
target_link_libraries(int2 dsm)

target_link_libraries(float1 dsm)
target_link_libraries(float2 dsm)
