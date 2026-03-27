cmake_minimum_required(VERSION 4.1)
project(qadra VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(Qt6_DIR C:/Qt/6.10.2/msvc2022_64/lib/cmake/Qt6)
set(OpenCASCADE_DIR "D:/3rd-party/opencascade-7.9.3-vc14-64/cmake")

find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGLWidgets Svg SvgWidgets)
find_package(OpenCASCADE REQUIRED CONFIG)

qt_standard_project_setup()

set(QADRA_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/cad
  ${CMAKE_SOURCE_DIR}/command
  ${CMAKE_SOURCE_DIR}/core
  ${CMAKE_SOURCE_DIR}/entity
  ${CMAKE_SOURCE_DIR}/kernel
  ${CMAKE_SOURCE_DIR}/math
  ${CMAKE_SOURCE_DIR}/render
  ${CMAKE_SOURCE_DIR}/spatial
  ${CMAKE_SOURCE_DIR}/tool
  ${CMAKE_SOURCE_DIR}/ui
)

add_subdirectory(vendor)
add_subdirectory(gl)
add_subdirectory(math)
add_subdirectory(core)
add_subdirectory(entity)
add_subdirectory(spatial)
add_subdirectory(cad)
add_subdirectory(kernel)
add_subdirectory(render)
add_subdirectory(command)
add_subdirectory(tool)
add_subdirectory(ui)

set(app_icon_resource_windows "")
if(WIN32)
  set(app_icon_resource_windows qadra.rc)
endif()

qt_add_executable(qadra main.cpp
        ${app_icon_resource_windows}
)

qt_add_resources(qadra "app_icons"
        PREFIX "/icons"
        FILES
        qadra-icon.svg
)

qt_add_resources(qadra "app_assets"
        PREFIX "/assets"
        FILES
        qadra-splash-dark.svg
        qadra-splash-light.svg
)

qt_add_resources(qadra "app_draw_assets"
        PREFIX "/assets"
        BASE "${CMAKE_SOURCE_DIR}/assets"
        FILES
        assets/draw-arc.svg
        assets/draw-ellipse.svg
        assets/draw-line.svg
        assets/draw-text.svg
)

target_include_directories(qadra PRIVATE
  ${QADRA_INCLUDE_DIRS}
)

target_link_libraries(qadra PRIVATE
        Qt6::Widgets
        Qt6::Svg
        Qt6::SvgWidgets
        qadra_ui
)

set_target_properties(qadra PROPERTIES
        WIN32_EXECUTABLE ON
        MACOSX_BUNDLE ON
)

add_custom_target(copy_shaders ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:qadra>/shaders
)
add_dependencies(qadra copy_shaders)

if(WIN32)
  add_custom_command(TARGET qadra POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_RUNTIME_DLLS:qadra>
          $<TARGET_FILE_DIR:qadra>
          COMMAND_EXPAND_LISTS
  )

  add_custom_command(TARGET qadra POST_BUILD
          COMMAND ${CMAKE_COMMAND}
          -Dqadra_executable=$<TARGET_FILE:qadra>
          -Dqadra_destination=$<TARGET_FILE_DIR:qadra>
          -Dqadra_occt_root=${OpenCASCADE_INSTALL_PREFIX}
          -P ${CMAKE_SOURCE_DIR}/cmake/CopyOcctRuntimeDeps.cmake
          COMMAND_EXPAND_LISTS
  )
endif()

qt_generate_deploy_app_script(
        TARGET qadra
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
)

if(WIN32)
  add_custom_command(TARGET qadra POST_BUILD
          COMMAND ${CMAKE_COMMAND}
          -DQT_DEPLOY_PREFIX=$<TARGET_FILE_DIR:qadra>
          -DQT_DEPLOY_BIN_DIR=.
          -P ${deploy_script}
          COMMAND_EXPAND_LISTS
  )
endif()

install(TARGETS qadra
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(SCRIPT ${deploy_script})

find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
  file(GLOB SOURCES_ROOT ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/*.hpp)
  file(GLOB SOURCES_CAD ${CMAKE_SOURCE_DIR}/cad/*.cpp ${CMAKE_SOURCE_DIR}/cad/*.hpp)
  file(GLOB_RECURSE SOURCES_COMMAND ${CMAKE_SOURCE_DIR}/command/*.cpp ${CMAKE_SOURCE_DIR}/command/*.hpp)
  file(GLOB SOURCES_CORE ${CMAKE_SOURCE_DIR}/core/*.cpp ${CMAKE_SOURCE_DIR}/core/*.hpp)
  file(GLOB SOURCES_ENTITY ${CMAKE_SOURCE_DIR}/entity/*.cpp ${CMAKE_SOURCE_DIR}/entity/*.hpp)
  file(GLOB SOURCES_GL ${CMAKE_SOURCE_DIR}/gl/*.cpp ${CMAKE_SOURCE_DIR}/gl/*.hpp)
  file(GLOB_RECURSE SOURCES_KERNEL ${CMAKE_SOURCE_DIR}/kernel/*.cpp ${CMAKE_SOURCE_DIR}/kernel/*.hpp)
  file(GLOB SOURCES_MATH ${CMAKE_SOURCE_DIR}/math/*.cpp ${CMAKE_SOURCE_DIR}/math/*.hpp)
  file(GLOB SOURCES_RENDER ${CMAKE_SOURCE_DIR}/render/*.cpp ${CMAKE_SOURCE_DIR}/render/*.hpp)
  file(GLOB SOURCES_SPATIAL ${CMAKE_SOURCE_DIR}/spatial/*.cpp ${CMAKE_SOURCE_DIR}/spatial/*.hpp)
  file(GLOB SOURCES_TOOL ${CMAKE_SOURCE_DIR}/tool/*.cpp ${CMAKE_SOURCE_DIR}/tool/*.hpp)
  file(GLOB SOURCES_UI ${CMAKE_SOURCE_DIR}/ui/*.cpp ${CMAKE_SOURCE_DIR}/ui/*.hpp)

  set(ALL_SOURCES
    ${SOURCES_ROOT}
    ${SOURCES_CAD}
    ${SOURCES_COMMAND}
    ${SOURCES_CORE}
    ${SOURCES_ENTITY}
    ${SOURCES_GL}
    ${SOURCES_KERNEL}
    ${SOURCES_MATH}
    ${SOURCES_RENDER}
    ${SOURCES_SPATIAL}
    ${SOURCES_TOOL}
    ${SOURCES_UI}
  )

  add_custom_target(format
    COMMAND "${CLANG_FORMAT}" -i ${ALL_SOURCES}
    COMMENT "Formatting source files"
  )
endif()
