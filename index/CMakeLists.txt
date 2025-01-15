add_executable(build_test_index build_test_index.cpp)
target_include_directories(build_test_index PRIVATE ${CMAKE_SOURCE_DIR}/include/LZ/lz77index/)
target_link_libraries(build_test_index ctriepp malloc_count SLP ${CMAKE_SOURCE_DIR}/build/lib/liblz77.a ${CMAKE_SOURCE_DIR}/include/LZ/lz77index/libdivsufsort.a)
target_compile_options(build_test_index PRIVATE -Wno-c++11-narrowing -w)

#add_executable(build_index_prova build_index_prova.cpp)
#target_link_libraries(build_index_prova PUBLIC ${CMAKE_SOURCE_DIR}/include/LZ/lz77index/lib/liblz77.a)
#target_include_directories(build_index_prova PUBLIC ${CMAKE_SOURCE_DIR}/include/LZ/lz77index/)
#target_compile_options(build_index_prova PRIVATE -Wno-c++11-narrowing -w)