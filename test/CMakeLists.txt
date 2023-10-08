cmake_minimum_required( VERSION 3.10 )

set( TEST_EXECS
    test_pHuffTreeBuilder
    perftest_BitPacking
)

include_directories( /usr/local/include/ )
link_directories( /usr/local/lib/ )

add_executable( test_pHuffTreeBuilder test_pHuffTreeBuilder.cpp )
add_executable( perftest_BitPacking perftest_BitPacking.cpp )

target_include_directories( test_pHuffTreeBuilder PUBLIC ${CMAKE_SOURCE_DIR} )
target_include_directories( test_pHuffTreeBuilder PUBLIC ${CMAKE_SOURCE_DIR}/thirdparty/googletest/googletest/include )
target_include_directories( perftest_BitPacking PUBLIC ${CMAKE_SOURCE_DIR} )
target_include_directories( perftest_BitPacking PUBLIC ${CMAKE_SOURCE_DIR}/thirdparty/googletest/googletest/include )

target_link_libraries( test_pHuffTreeBuilder PUBLIC gtest )
target_link_libraries( perftest_BitPacking PUBLIC gtest )
target_link_libraries( perftest_BitPacking PUBLIC benchmark )
target_compile_options( perftest_BitPacking PUBLIC -mavx2 )
