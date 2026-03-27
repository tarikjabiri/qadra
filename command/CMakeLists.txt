add_library(qadra_command STATIC
  arc/Command.cpp
  arc/Command.hpp
  arc/State.hpp
  Context.hpp
  HistoryEntry.hpp
  line/Command.cpp
  line/Command.hpp
  line/State.hpp
  Manager.cpp
  Manager.hpp
  Output.hpp
  point/Expression.hpp
  point/Parser.cpp
  point/Parser.hpp
  point/Resolver.cpp
  point/Resolver.hpp
  PointerEvent.hpp
  Preview.hpp
  Registry.cpp
  Registry.hpp
  SessionCommand.hpp
  View.hpp
)

target_include_directories(qadra_command PUBLIC
  ${QADRA_INCLUDE_DIRS}
)

target_link_libraries(qadra_command PUBLIC
  glm
  qadra_cad
)
