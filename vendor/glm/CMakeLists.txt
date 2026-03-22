set(GLM_BUILD_TESTS OFF CACHE BOOL "Build GLM tests" FORCE)

FetchContent_Declare(
  glm
  GIT_REPOSITORY  https://github.com/g-truc/glm.git
  GIT_TAG         1.0.3
  GIT_SHALLOW     TRUE
  GIT_PROGRESS    TRUE
)

FetchContent_MakeAvailable(glm)
