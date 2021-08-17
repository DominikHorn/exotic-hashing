include(FetchContent)

set(FST_LIBRARY fst)
FetchContent_Declare(
  ${FST_LIBRARY}
  GIT_REPOSITORY https://github.com/DominikHorn/FST
  GIT_TAG 9dc43ee
)

FetchContent_MakeAvailable(${FST_LIBRARY})
