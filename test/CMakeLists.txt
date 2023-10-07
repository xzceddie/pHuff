cmake_minimum_required( VERSION 3.10 )

set( TEST_EXECS
    test_pHuffTreeBuilder
)

add_executable( test_pHuffTreeBuilder test_pHuffTreeBuilder.cpp )

target_include_directories( test_pHuffTreeBuilder PUBLIC ${CMAKE_SOURCE_DIR} )
target_include_directories( test_pHuffTreeBuilder PUBLIC ${CMAKE_SOURCE_DIR}/thirdparty/googletest/googletest/include )

target_link_libraries( test_pHuffTreeBuilder PUBLIC gtest )
