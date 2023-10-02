cmake_minimum_required( VERSION 3.10 )

add_library( pHuff SHARED ./seqAnalyzer.cpp ./pHuffCounter.cpp )

add_executable( pHuffExec main.cpp )

include_directories( ../include/ )
