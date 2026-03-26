qt_add_library(qadra_ui STATIC
  Canvas.cpp
  Canvas.hpp
  CmdLine.cpp
  CmdLine.hpp
  DrawToolBar.cpp
  DrawToolBar.hpp
  Window.cpp
  Window.hpp
)

target_include_directories(qadra_ui PUBLIC
  ${QADRA_INCLUDE_DIRS}
)

target_link_libraries(qadra_ui PUBLIC
  Qt6::Widgets
  Qt6::OpenGLWidgets
  qadra_cad
  qadra_core
  qadra_render
  glad
)
