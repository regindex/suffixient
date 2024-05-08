add_executable(pfp_suffixient pfp_suffixient.cpp)
target_link_libraries(pfp_suffixient common pfp pfp_iterator gsacak sdsl malloc_count)

add_executable(pfp_suffixient64 pfp_suffixient.cpp)
target_link_libraries(pfp_suffixient64 common pfp pfp_iterator gsacak64 sdsl malloc_count)
target_compile_options(pfp_suffixient64 PUBLIC -DM64)

add_executable(suffixient suffixient_onepass.cpp)
target_link_libraries(suffixient common pfp sdsl divsufsort divsufsort64)

add_executable(suffixient_twopass suffixient_twopass.cpp)
target_link_libraries(suffixient_twopass common pfp sdsl divsufsort divsufsort64)