FetchContent_Declare(
  glad
  GIT_REPOSITORY  https://github.com/Dav1dde/glad.git
  GIT_TAG         v2.0.8
  GIT_SHALLOW     TRUE
  GIT_PROGRESS    TRUE
  SOURCE_SUBDIR   cmake
)

FetchContent_MakeAvailable(glad)

glad_add_library(
  glad
  STATIC
  REPRODUCIBLE
  LOADER
  API
    gl:core=4.6
)
