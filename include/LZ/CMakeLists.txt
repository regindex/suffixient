# C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compilator flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wno-c++11-narrowing -DNDEBUG")

# List of source files for the library
set(SOURCE_FILES 
    lz77index/utils.cpp
    lz77index/mapper.cpp
    lz77index/static_selfindex.cpp
    lz77index/static_selfindex_none.cpp
    lz77index/static_selfindex_lz77.cpp
    lz77index/LZparser.cpp
    lz77index/LZ77.cpp
    lz77index/LZend.cpp
    lz77index/treelist.cpp
    lz77index/static_bitsequence.cpp
    lz77index/static_bitsequence_brw32.cpp
    lz77index/static_bitsequence_builder_brw32.cpp
    lz77index/static_permutation.cpp
    lz77index/static_permutation_mrrr.cpp
    lz77index/RMQ.cpp
    lz77index/RMQ_succinct.cpp
    lz77index/perm.cpp
    lz77index/deltacodes.cpp
    lz77index/patricia.cpp
    lz77index/patricia_node.cpp
    lz77index/bp_hb.cpp
    lz77index/dfuds.cpp
    lz77index/static_doublebitmap.cpp
    lz77index/static_doublebitmap_s.cpp
    lz77index/hash.cpp
    lz77index/utils_index.cpp
    lz77index/range.cpp
    lz77index/static_range.cpp
    lz77index/static_range_none.cpp
    lz77index/static_range_chazelle.cpp
    lz77index/static_sequence_wt.cpp
    lz77index/wt_node.cpp
    lz77index/perm2.cpp
    lz77index/static_permutation_mrrr2.cpp
    lz77index/static_selfindex_lzend.cpp
    lz77index/directcodes.cpp
    lz77index/bitrankw32int.cpp
)

# External library file
set(LIBRARY_FILES ${CMAKE_SOURCE_DIR}/lz77index/libdivsufsort.a)

# Set the output directory for static libraries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Create the static library
add_library(lz77 STATIC ${SOURCE_FILES})
# Link with external libraries
target_link_libraries(lz77 PUBLIC ${LIBRARY_FILES})