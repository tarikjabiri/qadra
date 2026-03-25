add_library(qadra_spatial STATIC
  SpatialIndex.cpp
  SpatialIndex.hpp
)

target_include_directories(qadra_spatial PUBLIC
  ${QADRA_INCLUDE_DIRS}
)

target_link_libraries(qadra_spatial PUBLIC
  Qt6::Core
  Boost::geometry
  Boost::headers
  glm
)
