include(FetchContent)

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