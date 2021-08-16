include(FetchContent)

set(FST_LIBRARY fst)
FetchContent_Declare(
  ${FST_LIBRARY}
  GIT_REPOSITORY https://github.com/christophanneser/FST.git
  GIT_TAG e4a0824
)

# fst CMakeLists.txt is broken, therefore can't rely on 'sane defaults'
# using MakeAvailable
FetchContent_GetProperties(${FST_LIBRARY})
string(TOLOWER "${FST_LIBRARY}" fstContentName)
if(NOT ${fstContentName}_POPULATED)
  # Fetch the content using previously declared details
  FetchContent_Populate(${FST_LIBRARY})

  # Bring the populated content into the build
  add_library(${FST_LIBRARY} INTERFACE)
  target_include_directories(${FST_LIBRARY} INTERFACE
    $<BUILD_INTERFACE:${${fstContentName}_SOURCE_DIR}/..>)
endif()
