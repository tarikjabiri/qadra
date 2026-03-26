add_library(qadra_cad STATIC
  DocumentChange.hpp
  Document.cpp
  Document.hpp
)

target_include_directories(qadra_cad PUBLIC
  ${QADRA_INCLUDE_DIRS}
)

target_link_libraries(qadra_cad PUBLIC
  Qt6::Core
  qadra_core
  qadra_entity
  qadra_spatial
)
