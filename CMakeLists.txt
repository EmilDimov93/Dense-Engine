cmake_minimum_required(VERSION 3.25)

project(DenseEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENGINE_SOURCES
    example/example.cpp

    src/client.cpp

    src/renderer/init.cpp
    src/renderer/runtime.cpp

    src/scene/scene.cpp
    src/scene/tlas.cpp
    src/scene/mesh.cpp
    src/scene/modelLoader.cpp

    src/window/window.cpp
)

add_executable(DenseEngine ${ENGINE_SOURCES})

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
    d3d12
    dxgi
    dxguid
)

find_program(DXC_EXECUTABLE dxc REQUIRED)

add_custom_command(
    TARGET DenseEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:DenseEngine>/shaders
    COMMAND ${DXC_EXECUTABLE} -T lib_6_3 -Fo $<TARGET_FILE_DIR:DenseEngine>/shaders/shader.cso ${CMAKE_SOURCE_DIR}/src/renderer/shaders/shader.hlsl
    COMMENT "Compiling raytracing shaders"
)