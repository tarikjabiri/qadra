add_library(qadra_math STATIC
  BoxAABB.cpp
  BoxAABB.hpp
  BoxOBB.cpp
  BoxOBB.hpp
  Grid.cpp
  Grid.hpp
)

target_include_directories(qadra_math PUBLIC
  ${QADRA_INCLUDE_DIRS}
)

target_link_libraries(qadra_math PUBLIC
  Qt6::Core
  glm
)
