add_executable(pfp_suffixient pfp_suffixient.cpp)
target_link_libraries(pfp_suffixient common pfp pfp_iterator gsacak sdsl malloc_count)

add_executable(pfp_suffixient64 pfp_suffixient.cpp)
target_link_libraries(pfp_suffixient64 common pfp pfp_iterator gsacak64 sdsl malloc_count)
target_compile_options(pfp_suffixient64 PUBLIC -DM64)

add_executable(one-pass one_pass.cpp)
target_link_libraries(one-pass common sdsl divsufsort divsufsort64)

add_executable(suffixient linear_time.cpp)
target_link_libraries(suffixient common sdsl divsufsort divsufsort64)

add_executable(test test.cpp)
target_link_libraries(test common sdsl divsufsort divsufsort64)

add_executable(fm fm.cpp)
target_link_libraries(fm common sdsl divsufsort divsufsort64)

add_executable(plc plain_lc.cpp)
target_link_libraries(plc common sdsl divsufsort divsufsort64)

add_executable(lc lc.cpp)
target_link_libraries(lc common sdsl divsufsort divsufsort64)

#add_executable(stats stats.cpp)
#target_link_libraries(suffixient common sdsl divsufsort divsufsort64)
