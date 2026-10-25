cmake_minimum_required(VERSION 3.25)

project(DenseEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENGINE_SOURCES
    src/main.cpp

    src/client.cpp

    src/renderer/init.cpp
    src/renderer/runtime.cpp

    src/scene/scene.cpp
    src/scene/tlas.cpp
    src/scene/mesh.cpp
    src/scene/modelLoader.cpp

    src/ui/ui.cpp

    src/window/window.cpp
)

add_library(imgui STATIC
    ext/imgui/imgui.cpp
    ext/imgui/imgui_draw.cpp
    ext/imgui/imgui_tables.cpp
    ext/imgui/imgui_widgets.cpp
    ext/imgui/backends/imgui_impl_win32.cpp
    ext/imgui/backends/imgui_impl_dx12.cpp
)
target_include_directories(imgui PUBLIC ext/imgui)

add_executable(DenseEngine ${ENGINE_SOURCES} assets/DenseEngine.rc)

target_include_directories(DenseEngine PRIVATE src)

target_compile_definitions(DenseEngine PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
)

target_compile_options(DenseEngine PRIVATE
    /W4 /permissive- /Zc:preprocessor /EHsc /MP
    $<$<CONFIG:Release>:/O2 /Oi /Ot /Gy /fp:fast /arch:AVX2>
)

set_target_properties(DenseEngine PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

target_link_libraries(DenseEngine PRIVATE
    imgui
    d3d12
    dxgi
    dxguid
)

find_program(DXC_EXECUTABLE dxc REQUIRED)

set(SHADER_SOURCE ${CMAKE_SOURCE_DIR}/src/renderer/shaders/shader.hlsl)
set(SHADER_HEADER ${CMAKE_BINARY_DIR}/generated/shader.h)

add_custom_command(
    OUTPUT ${SHADER_HEADER}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/generated
    COMMAND ${DXC_EXECUTABLE} -T lib_6_3 -Fh ${SHADER_HEADER} -Vn shaderBytecode ${SHADER_SOURCE}
    DEPENDS ${SHADER_SOURCE}
    COMMENT "Compiling raytracing shaders"
)

target_sources(DenseEngine PRIVATE ${SHADER_HEADER})
target_include_directories(DenseEngine PRIVATE ${CMAKE_BINARY_DIR}/generated)