add_library(qadra_entity STATIC
  Arc.cpp
  Arc.hpp
  Entity.cpp
  Entity.hpp
  EntityType.hpp
  Ellipse.cpp
  Ellipse.hpp
  Line.cpp
  Line.hpp
  Text.cpp
  Text.hpp
)

target_include_directories(qadra_entity PUBLIC
  ${QADRA_INCLUDE_DIRS}
)

target_link_libraries(qadra_entity PUBLIC
  Qt6::Core
  qadra_core
  qadra_math
  glm
)
