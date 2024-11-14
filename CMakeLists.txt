cmake_minimum_required(VERSION 3.27.0)

project(EmbedLog VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(EmbedYAML STATIC)

target_sources(EmbedYAML PRIVATE
    "src/EmbedYAML.cpp"
)

target_include_directories(EmbedYAML PUBLIC
    "include"
)

target_link_libraries(EmbedYAML PUBLIC
    yaml
)

add_subdirectory(external/libyaml)
