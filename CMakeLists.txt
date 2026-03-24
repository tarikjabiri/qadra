cmake_minimum_required(VERSION 4.1)
project(qadra VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(Qt6_DIR C:/Qt/6.10.2/msvc2022_64/lib/cmake/Qt6)
set(OpenCASCADE_DIR "D:/3rd-party/opencascade-7.9.3-vc14-64/cmake")

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets Svg SvgWidgets)
find_package(OpenCASCADE REQUIRED CONFIG)

add_subdirectory(vendor)

qt_standard_project_setup()

set(app_icon_resource_windows "")
if(WIN32)
  set(app_icon_resource_windows qadra.rc)
endif()

qt_add_executable(qadra main.cpp
        Window.cpp
        Window.hpp
        Canvas.cpp
        Canvas.hpp
        Buffer.cpp
        Buffer.hpp
        VertexArray.cpp
        VertexArray.hpp
        Shader.cpp
        Shader.hpp
        Program.cpp
        Program.hpp
        Texture.cpp
        Texture.hpp
        core/Camera.cpp
        core/Camera.hpp
        core/Font.cpp
        core/Font.hpp
        math/Grid.cpp
        math/Grid.hpp
        GridPass.cpp
        GridPass.hpp
        ${app_icon_resource_windows}
        core/FontEngine.cpp
        core/FontEngine.hpp
        core/TextShaper.cpp
        core/TextShaper.hpp
        core/ShapedGlyph.hpp
        core/GlyphAtlas.cpp
        core/GlyphAtlas.hpp
        core/GlyphData.hpp
        math/BoxAabb.cpp
        math/BoxAABB.hpp
        math/BoxOBB.cpp
        math/BoxOBB.hpp
        core/Handle.hpp
        core/Handle.cpp
        entity/Entity.cpp
        entity/Entity.hpp
        entity/Line.cpp
        entity/Line.hpp
        cad/Document.cpp
        cad/Document.hpp
        render/LinePass.cpp
        render/LinePass.hpp
        entity/EntityType.hpp
        render/Renderer.cpp
        render/Renderer.hpp
        spatial/SpatialIndex.cpp
        spatial/SpatialIndex.hpp
        render/GeometryCache.cpp
        render/GeometryCache.hpp
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

target_include_directories(qadra PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/cad
  ${CMAKE_SOURCE_DIR}/core
  ${CMAKE_SOURCE_DIR}/entity
  ${CMAKE_SOURCE_DIR}/math
  ${CMAKE_SOURCE_DIR}/render
  ${CMAKE_SOURCE_DIR}/spatial
)

target_link_libraries(qadra PRIVATE
        Qt6::Widgets
        Qt6::OpenGLWidgets
        Qt6::Svg
        Qt6::SvgWidgets
        Freetype::Freetype
        harfbuzz
        msdf-atlas-gen
        glad
        glm
        Boost::geometry
        Boost::headers
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

install(TARGETS qadra
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
        TARGET qadra
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
  file(GLOB SOURCES_ROOT ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/*.hpp)
  file(GLOB SOURCES_CAD ${CMAKE_SOURCE_DIR}/cad/*.cpp ${CMAKE_SOURCE_DIR}/cad/*.hpp)
  file(GLOB SOURCES_CORE ${CMAKE_SOURCE_DIR}/core/*.cpp ${CMAKE_SOURCE_DIR}/core/*.hpp)
  file(GLOB SOURCES_ENTITY ${CMAKE_SOURCE_DIR}/entity/*.cpp ${CMAKE_SOURCE_DIR}/entity/*.hpp)
  file(GLOB SOURCES_MATH ${CMAKE_SOURCE_DIR}/math/*.cpp ${CMAKE_SOURCE_DIR}/math/*.hpp)
  file(GLOB SOURCES_RENDER ${CMAKE_SOURCE_DIR}/render/*.cpp ${CMAKE_SOURCE_DIR}/render/*.hpp)
  file(GLOB SOURCES_SPATIAL ${CMAKE_SOURCE_DIR}/spatial/*.cpp ${CMAKE_SOURCE_DIR}/spatial/*.hpp)

  set(ALL_SOURCES
    ${SOURCES_ROOT}
    ${SOURCES_CAD}
    ${SOURCES_CORE}
    ${SOURCES_ENTITY}
    ${SOURCES_MATH}
    ${SOURCES_RENDER}
    ${SOURCES_SPATIAL}
  )

  add_custom_target(format
    COMMAND "${CLANG_FORMAT}" -i ${ALL_SOURCES}
    COMMENT "Formatting source files"
  )
endif()
