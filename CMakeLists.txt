cmake_minimum_required(VERSION 2.6)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

#set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/.. )

project (suffixient)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(~/include)  #SDSL headers
LINK_DIRECTORIES(~/lib)         #SDSL libs

message("Building in ${CMAKE_BUILD_TYPE} mode")

set(CMAKE_CXX_FLAGS "--std=c++14")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb -g")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -fstrict-aliasing -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -ggdb -Ofast -fstrict-aliasing -march=native")

add_executable(suffixient suffixient.cpp)
TARGET_LINK_LIBRARIES(suffixient sdsl)
TARGET_LINK_LIBRARIES(suffixient divsufsort)
TARGET_LINK_LIBRARIES(suffixient divsufsort64)
