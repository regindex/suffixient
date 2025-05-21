include(FetchContent)

## Add zlib
FetchContent_Declare(
  zlib
  URL https://zlib.net/zlib-1.3.1.tar.gz
)
#FetchContent_MakeAvailable(zlib)
FetchContent_GetProperties(zlib)
if(NOT zlib_POPULATED)
  FetchContent_Populate(zlib)

  add_subdirectory(${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
  endif()


## Add pfp-thresholds
FetchContent_Declare(
  pfplcp
  GIT_REPOSITORY https://github.com/maxrossi91/pfp-thresholds.git
  GIT_TAG 0716791f7feb8bb5eec657557279034726443647
)

FetchContent_GetProperties(pfplcp)
if(NOT pfplcp_POPULATED)
  FetchContent_Populate(pfplcp)

  add_subdirectory(${pfplcp_SOURCE_DIR} ${pfplcp_BINARY_DIR})
  endif()