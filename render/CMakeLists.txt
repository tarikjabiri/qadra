add_library(qadra_render STATIC
  GeometryCache.cpp
  GeometryCache.hpp
  GridPass.cpp
  GridPass.hpp
  LinePass.cpp
  LinePass.hpp
  RenderPass.cpp
  RenderPass.hpp
  Renderer.cpp
  Renderer.hpp
  TextPass.cpp
  TextPass.hpp
)

target_include_directories(qadra_render PUBLIC
  ${QADRA_INCLUDE_DIRS}
)

target_link_libraries(qadra_render PUBLIC
  Qt6::Core
  qadra_gl
  qadra_cad
  qadra_core
  qadra_math
  glm
)
