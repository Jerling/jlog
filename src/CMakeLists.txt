aux_source_directory(. DIR_LIB_SRCS)
include_directories(../jlog)
add_library(jlog SHARED ${DIR_LIB_SRCS})
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
find_package(Threads)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} tcmalloc)
set(CMAKE_CXX_FLAGS
  -lpthread
  )
