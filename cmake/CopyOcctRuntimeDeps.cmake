if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()

if(NOT DEFINED qadra_executable OR qadra_executable STREQUAL "")
  message(FATAL_ERROR "qadra_executable is required")
endif()

if(NOT DEFINED qadra_destination OR qadra_destination STREQUAL "")
  message(FATAL_ERROR "qadra_destination is required")
endif()

if(NOT DEFINED qadra_occt_root OR qadra_occt_root STREQUAL "")
  return()
endif()

set(qadra_occt_root_normalized "${qadra_occt_root}")
cmake_path(NORMAL_PATH qadra_occt_root_normalized)
cmake_path(GET qadra_occt_root_normalized PARENT_PATH qadra_occt_parent)
set(qadra_occt_thirdparty_root "${qadra_occt_parent}/3rdparty-vc14-64")

file(GLOB qadra_occt_thirdparty_bin_dirs LIST_DIRECTORIES true
     "${qadra_occt_thirdparty_root}/*/bin")

set(qadra_runtime_search_dirs
    "${qadra_destination}"
    ${qadra_occt_thirdparty_bin_dirs})

file(GET_RUNTIME_DEPENDENCIES
     EXECUTABLES "${qadra_executable}"
     RESOLVED_DEPENDENCIES_VAR qadra_resolved_runtime_deps
     UNRESOLVED_DEPENDENCIES_VAR qadra_unresolved_runtime_deps
     DIRECTORIES ${qadra_runtime_search_dirs}
     PRE_EXCLUDE_REGEXES "api-ms-.*" "ext-ms-.*"
     POST_EXCLUDE_REGEXES ".*/Windows/System32/.*" ".*/Windows/WinSxS/.*")

set(qadra_occt_runtime_dlls "")
foreach(qadra_dep IN LISTS qadra_resolved_runtime_deps)
  set(qadra_dep_normalized "${qadra_dep}")
  cmake_path(NORMAL_PATH qadra_dep_normalized)

  cmake_path(IS_PREFIX qadra_occt_root_normalized "${qadra_dep_normalized}" NORMALIZE
             qadra_is_occt_dep)
  cmake_path(IS_PREFIX qadra_occt_thirdparty_root "${qadra_dep_normalized}" NORMALIZE
             qadra_is_occt_thirdparty_dep)

  if(qadra_is_occt_dep OR qadra_is_occt_thirdparty_dep)
    list(APPEND qadra_occt_runtime_dlls "${qadra_dep}")
  endif()
endforeach()

list(REMOVE_DUPLICATES qadra_occt_runtime_dlls)

if(qadra_occt_runtime_dlls)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${qadra_occt_runtime_dlls}
            "${qadra_destination}"
    COMMAND_ERROR_IS_FATAL ANY)
endif()
