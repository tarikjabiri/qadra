add_library(qadra_kernel STATIC
  ArcBuilder.hpp
  EllipseBuilder.hpp
  occt/ArcBuilder.cpp
  occt/EllipseBuilder.cpp
)

target_include_directories(qadra_kernel PUBLIC
  ${QADRA_INCLUDE_DIRS}
)

target_link_libraries(qadra_kernel PUBLIC
  qadra_math
  TKG2d
  TKGeomBase
)
