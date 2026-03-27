add_library(qadra_math STATIC
  Arc.cpp
  Arc.hpp
  BoxAABB.cpp
  BoxAABB.hpp
  BoxOBB.cpp
  BoxOBB.hpp
  Ellipse.cpp
  Ellipse.hpp
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
