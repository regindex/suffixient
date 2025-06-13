add_executable(pfp_suffixient pfp_suffixient.cpp)
target_link_libraries(pfp_suffixient common pfp pfp_iterator gsacak sdsl malloc_count)

add_executable(pfp_suffixient64 pfp_suffixient.cpp)
target_link_libraries(pfp_suffixient64 common pfp pfp_iterator gsacak64 sdsl malloc_count)
target_compile_options(pfp_suffixient64 PUBLIC -DM64)

add_executable(one-pass one_pass.cpp)
target_link_libraries(one-pass common sdsl divsufsort divsufsort64)

add_executable(suffixient linear_time.cpp)
target_link_libraries(suffixient common sdsl divsufsort divsufsort64)

add_executable(suffixiency_test test.cpp)
target_link_libraries(suffixiency_test common sdsl divsufsort divsufsort64)

#add_executable(stats stats.cpp)
#target_link_libraries(suffixient common sdsl divsufsort divsufsort64)