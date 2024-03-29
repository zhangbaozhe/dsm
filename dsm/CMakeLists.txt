set(DSM_INCLUDE_DIR
  include
  include/dsm
)

set(DSM_SRC
  
)

include_directories(${DSM_INCLUDE_DIR})

add_executable(main
  src/main.cpp
)