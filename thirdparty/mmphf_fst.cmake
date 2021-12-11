include(FetchContent)

set(MMPHF_FST_LIBRARY mmphf_fst)
FetchContent_Declare(
  ${MMPHF_FST_LIBRARY}
  GIT_REPOSITORY https://github.com/DominikHorn/mmphf_fst
  GIT_TAG ef5eec2 
)

FetchContent_MakeAvailable(${MMPHF_FST_LIBRARY})
