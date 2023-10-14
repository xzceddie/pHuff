cmake_minimum_required( VERSION 3.10 )

# add_library( pHuff SHARED ./seqAnalyzer.cpp )
add_library( pHuffEnc SHARED pHuffEncoder.cpp )

add_executable( pHuffExec main.cpp )
target_compile_options( pHuffExec PUBLIC -mavx2 )

include_directories( ${CMAKE_SOURCE_DIR}/include/ )
include_directories( ${CMAKE_SOURCE_DIR} )

target_link_libraries( pHuffEnc PUBLIC pthread )
