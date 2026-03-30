add_library(qadra_gl STATIC
  Buffer.cpp
  Buffer.hpp
  Program.cpp
  Program.hpp
  Shader.cpp
  Shader.hpp
  Texture.cpp
  Texture.hpp
  VertexArray.cpp
  VertexArray.hpp
  VertexLayout.hpp
)

target_include_directories(qadra_gl PUBLIC
  ${QADRA_INCLUDE_DIRS}
)

target_link_libraries(qadra_gl PUBLIC
  Qt6::Core
  glad
  glm
)
