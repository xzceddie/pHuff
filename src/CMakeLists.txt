cmake_minimum_required( VERSION 3.10 )

add_library( pHuff SHARED ./seqAnalyzer.cpp )

add_executable( pHuffExec main.cpp )

include_directories( ${CMAKE_SOURCE_DIR}/include/ )
include_directories( ${CMAKE_SOURCE_DIR} )
