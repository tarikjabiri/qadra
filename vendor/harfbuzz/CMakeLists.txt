set(HB_BUILD_SUBSET OFF CACHE BOOL "" FORCE)
set(HB_BUILD_UTILS OFF CACHE BOOL "" FORCE)
set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)

FetchContent_Declare(
  harfbuzz
  GIT_REPOSITORY  https://github.com/harfbuzz/harfbuzz.git
  GIT_TAG         13.1.1
  GIT_SHALLOW     TRUE
  GIT_PROGRESS    TRUE
)

FetchContent_MakeAvailable(harfbuzz)
