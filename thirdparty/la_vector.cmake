include(FetchContent)

set(LA_VECTOR_LIBRARY la_vector)
FetchContent_Declare(
  ${LA_VECTOR_LIBRARY}
  GIT_REPOSITORY https://github.com/DominikHorn/la_vector.git
  GIT_TAG 5efa77b
  )

FetchContent_MakeAvailable(${LA_VECTOR_LIBRARY})
