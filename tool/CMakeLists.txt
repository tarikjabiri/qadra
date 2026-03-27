add_library(qadra_tool STATIC
  CommandResult.hpp
  LineTool.cpp
  LineTool.hpp
  Tool.hpp
  ToolCommand.hpp
  ToolContext.hpp
  ToolEventResult.hpp
  ToolKind.hpp
  ToolManager.cpp
  ToolManager.hpp
  ToolPointerEvent.hpp
  ToolPreview.hpp
)

target_include_directories(qadra_tool PUBLIC
  ${QADRA_INCLUDE_DIRS}
)

target_link_libraries(qadra_tool PUBLIC
  qadra_cad
  qadra_command
  qadra_core
)
