# the minimum version of CMake.
cmake_minimum_required(VERSION 3.5)
project(hidumper_target)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_SYSTEM_NAME MATCHES "OHOS")
  set(CMAKE_FIND_ROOT_PATH ${NATIVERENDER_ROOT_PATH})
  include_directories(${NATIVERENDER_ROOT_PATH})
endif()

add_library(hidumper_target SHARED hidumper_target.cpp)
# libjsvm.so is deliberately NOT linked: the 32-bit board image does not ship it and a
# DT_NEEDED entry would break the whole module load. It is dlopen'ed at runtime instead.
target_link_libraries(hidumper_target PUBLIC libace_napi.z.so libhilog_ndk.z.so)
