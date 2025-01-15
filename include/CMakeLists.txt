#add_subdirectory(pfp_iterator)
#target_include_directories(pfp_iterator PUBLIC pfp_iterator)

add_subdirectory(ctriepp)
target_include_directories(ctriepp PUBLIC ctriepp)

add_subdirectory(SLP)
target_include_directories(SLP PUBLIC SLP)

add_subdirectory(LZ)
#target_include_directories(LZ PUBLIC LZ)