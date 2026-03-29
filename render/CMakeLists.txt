add_library(qadra_render STATIC
  ArcInstanceBuilder.cpp
  ArcInstanceBuilder.hpp
  ArcPass.cpp
  ArcPass.hpp
  CircleInstanceBuilder.cpp
  CircleInstanceBuilder.hpp
  EllipseInstanceBuilder.cpp
  EllipseInstanceBuilder.hpp
  EllipsePass.cpp
  EllipsePass.hpp
  GridPass.cpp
  GridPass.hpp
  LinePass.cpp
  LinePass.hpp
  LWPolylineGeometryBuilder.cpp
  LWPolylineGeometryBuilder.hpp
  ManagedBuffer.hpp
  PreviewRenderer.cpp
  PreviewRenderer.hpp
  RenderScene.cpp
  RenderScene.hpp
  RenderPass.cpp
  RenderPass.hpp
  Renderer.cpp
  Renderer.hpp
  SlotAllocator.hpp
  TextPass.cpp
  TextPass.hpp
  VertexBatch.hpp
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
