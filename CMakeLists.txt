cmake_minimum_required( VERSION 3.10 )

project( pHuff )

set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

add_subdirectory( ./src/ )
add_subdirectory( ./include/ )
add_subdirectory( ./test/ )
add_subdirectory( ./thirdparty/googletest/ )

