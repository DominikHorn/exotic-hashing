include(FetchContent)

set(HOT_LIBRARY hot-single-threaded-lib)
FetchContent_Declare(
  hot
  GIT_REPOSITORY https://github.com/DominikHorn/hot.git
  GIT_TAG 7c4e62b
  )

FetchContent_MakeAvailable(hot)
