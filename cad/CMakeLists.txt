add_library(qadra_cad STATIC
  DocumentChange.hpp
  EntitySnapshot.cpp
  EntitySnapshot.hpp
  Document.cpp
  Document.hpp
  history/DocumentEditor.cpp
  history/DocumentEditor.hpp
)

target_include_directories(qadra_cad PUBLIC
  ${QADRA_INCLUDE_DIRS}
)

target_link_libraries(qadra_cad PUBLIC
  Qt6::Core
  Qt6::Widgets
  qadra_core
  qadra_entity
  qadra_spatial
)
