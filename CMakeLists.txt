cmake_minimum_required(VERSION 3.14)
project(enen CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# IntgrNN library (installed via ./setup)
set(INTGRNN_DIR ${CMAKE_SOURCE_DIR}/external/intgr_nn)

if(NOT EXISTS ${INTGRNN_DIR}/include)
    message(FATAL_ERROR
        "IntgrNN not found. Run ./setup first.\n"
        "See README.md for details.")
endif()

include_directories(${INTGRNN_DIR}/include)
add_library(intgr_nn STATIC IMPORTED)
set_target_properties(intgr_nn PROPERTIES
    IMPORTED_LOCATION ${INTGRNN_DIR}/lib/libintgr_nn.a
)

# Project includes
include_directories(${CMAKE_SOURCE_DIR}/include)

# Main demo executable
add_executable(enen
    src/main.cpp
    src/game.cpp
    src/renderer.cpp
)
target_link_libraries(enen intgr_nn pthread)
target_compile_options(enen PRIVATE -Wall -Wextra -Wpedantic)

# Auto-run for video recording (outputs asciinema format)
add_executable(enen-autorun
    src/main_autorun.cpp
)
target_link_libraries(enen-autorun intgr_nn)
target_compile_options(enen-autorun PRIVATE -Wall -Wextra)

# Tests
add_executable(enen-net-test src/net_test.cpp)
target_link_libraries(enen-net-test intgr_nn)
target_compile_options(enen-net-test PRIVATE -Wall -Wextra)

add_executable(enen-game-test
    src/game_test.cpp
    src/game.cpp
)
target_link_libraries(enen-game-test intgr_nn)
target_compile_options(enen-game-test PRIVATE -Wall -Wextra)
