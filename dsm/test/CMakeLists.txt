add_executable(allocation1
  allocation1.cpp
)
add_executable(allocation2
  allocation2.cpp
)

target_link_libraries(allocation1 dsm)
target_link_libraries(allocation2 dsm)
