set(SLP_SOURCES slp/RePairSLPIndex.h)

add_library(SLP OBJECT ${SLP_SOURCES})

target_include_directories(SLP PUBLIC ./slp)
target_include_directories(SLP PUBLIC ./wslp)
target_include_directories(SLP PUBLIC ./libcds/includes)

target_link_libraries(SLP PUBLIC ${CMAKE_SOURCE_DIR}/include/SLP/libcds/lib/libcds.a)