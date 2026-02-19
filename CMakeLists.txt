# THIS IS A TEMPORARY ROOT CMAKELISTS.TXT WHILE FILES ARE BEING REFACTORED
cmake_minimum_required(VERSION 3.16)

project(Skewer LANGUAGES CXX)

# Set C++ Standard (C++17 is good for modern features)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Fixes CMake FetchContent warning
# (https://cmake.org/cmake/help/latest/policy/CMP0135.html)
cmake_policy(SET CMP0135 NEW)

# Use system-installed OpenEXR and Imath (install via apt-get or brew)
find_package(Imath REQUIRED)
find_package(OpenEXR REQUIRED)
find_package(ZLIB REQUIRED) # Apparently zlib is sometimes required

# Protobuf stuff
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(RENDERER_PROTO "${CMAKE_SOURCE_DIR}/api/proto/renderer/v1/renderer.proto")
set(PROTO_IMPORT_DIR "${CMAKE_SOURCE_DIR}/api/proto")
set(GEN_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${GEN_DIR})

add_custom_command(
    OUTPUT "${GEN_DIR}/renderer/v1/renderer.pb.cc"
           "${GEN_DIR}/renderer/v1/renderer.pb.h"
           "${GEN_DIR}/renderer/v1/renderer.grpc.pb.cc"
           "${GEN_DIR}/renderer/v1/renderer.grpc.pb.h"
    COMMAND protobuf::protoc
    ARGS --cpp_out "${GEN_DIR}"
         --grpc_out "${GEN_DIR}"
         --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
         -I "${PROTO_IMPORT_DIR}"
         "${RENDERER_PROTO}"
    DEPENDS "${RENDERER_PROTO}"
)

# Create a library for the generated code
add_library(renderer_grpc_proto
    "${GEN_DIR}/renderer/v1/renderer.pb.cc"
    "${GEN_DIR}/renderer/v1/renderer.grpc.pb.cc"
)
target_link_libraries(renderer_grpc_proto PUBLIC gRPC::grpc++ protobuf::libprotobuf)
target_include_directories(renderer_grpc_proto PUBLIC "${GEN_DIR}")

# Fetch nlohmann/json for scene loading
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)

FetchContent_MakeAvailable(nlohmann_json)
# Define Source Files
# list every .cpp file here. no need for .h
set(SOURCES
    apps/cli/main.cc
    src/session/render_session.cc
    src/scene/scene.cc
    src/film/film.cc
    src/film/image_buffer.cc
    src/integrators/path_trace.cc
    src/integrators/normals.cc
    src/accelerators/bvh.cc
    src/scene/light.cc
    src/io/obj_loader.cc
    src/materials/bsdf.cc
)

# Create Executable
add_executable(skewer-render ${SOURCES})

# Include directories tell CMake where to look for header files
target_include_directories(skewer-render
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/external
)

# Link libraries
target_link_libraries(skewer-render
    PRIVATE
    nlohmann_json::nlohmann_json
    Imath::Imath
    OpenEXR::OpenEXR
    ZLIB::ZLIB
    renderer_grpc_proto
)
