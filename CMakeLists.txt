cmake_minimum_required( VERSION 3.10 )

project( pHuff )

set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_OUTPUT_COMPILE_COMMANDS ON )

add_subdirectory( ./src/ )
add_subdirectory( ./include/ )

