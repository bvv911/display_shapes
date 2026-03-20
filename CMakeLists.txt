cmake_minimum_required(VERSION 3.18)
project(CMakeSFMLProject LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable(main src/main.cpp)
target_compile_features(main PRIVATE cxx_std_17)
target_link_libraries(main PRIVATE SFML::Graphics)

target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic -O3)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)

file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.hpp *.h)
list(FILTER ALL_SOURCE_FILES EXCLUDE REGEX "/build/")
add_custom_target(
    clang-format
    COMMAND clang-format --verbose -i ${ALL_SOURCE_FILES}
    )
