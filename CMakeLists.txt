cmake_minimum_required(VERSION 3.28)

add_subdirectory(external/libyaml)

project(EmbedLog)
add_library(EmbedYAML STATIC)

target_sources(EmbedYAML PRIVATE
    "src/EmbedYAML.cpp"
)

target_include_directories(EmbedYAML PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(EmbedYAML PUBLIC
    yaml
)

option(BUILD_EXAMPLES "Build examples" OFF)
if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
