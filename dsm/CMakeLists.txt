add_compile_options(-Wno-deprecated-declarations)



set(DSM_INCLUDE_DIR
  include
  include/dsm
)

set(DSM_SRC
  # src/request.cpp
  # src/response.cpp
  src/stringutils.cpp
  # src/client.cpp
  src/object.cpp
  src/manager.cpp
)


add_library(dsm
  ${DSM_SRC}
)
target_link_libraries(dsm
  pthread
  z
)

include_directories(
  ${DSM_INCLUDE_DIR}
  external
  ${FOLLY_INCLUDE_DIR}
)

add_executable(main
  main.cpp
)
target_link_libraries(main
  dsm
)
add_executable(param_server
  param_server.cpp
)
target_link_libraries(param_server
  pthread
)

add_subdirectory(test)