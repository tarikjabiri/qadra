add_library(qadra_core STATIC
  Camera.cpp
  Camera.hpp
  CameraController.cpp
  CameraController.hpp
  Font.cpp
  Font.hpp
  FontEngine.cpp
  FontEngine.hpp
  GlyphAtlas.cpp
  GlyphAtlas.hpp
  GlyphData.hpp
  Handle.cpp
  Handle.hpp
  ShapedGlyph.hpp
  TextLayout.hpp
  TextShaper.cpp
  TextShaper.hpp
)

target_include_directories(qadra_core PUBLIC
  ${QADRA_INCLUDE_DIRS}
)

target_link_libraries(qadra_core PUBLIC
  Qt6::Core
  qadra_gl
  qadra_math
  Freetype::Freetype
  harfbuzz
  msdf-atlas-gen
  glm
)
