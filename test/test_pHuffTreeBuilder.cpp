#include <gtest/gtest.h>
#include <include/pHuffTreeBuilder.h>
#include <include/pHuffCounter.h>
#include <iostream>
#include <array>
#include <vector>
#include <ranges>
#include <cstdint>


static const std::array<std::uint8_t, 5> TEST_BYTE_STREAM_1 { 0, 1, 1, 1, 0 };
static const std::vector<std::uint8_t> TEST_BYTE_STREAM_2 { 0, 1, 1, 1, 0 };
static const std::uint8_t TEST_BYTE_STREAM_3[5] = { 0, 1, 1, 1, 0 };

static const std::array<std::uint8_t, 5> TEST_BYTE_STREAM_4 { 0, 1, 2, 3, 4 };
static const std::vector<std::uint8_t> TEST_BYTE_STREAM_5 { 0, 1, 2, 3, 4 };
static const std::uint8_t TEST_BYTE_STREAM_6[5] = { 0, 1, 2, 3, 4 };

class TestHuffTreBuilder
    : public ::testing::Test
{
};


TEST_F( TestHuffTreBuilder, CounterArray )
{
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_1 )[0], 2 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_1 )[1], 3 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_4 )[0], 1 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_4 )[1], 1 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_4 )[2], 1 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_4 )[3], 1 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_4 )[4], 1 );
}

TEST_F( TestHuffTreBuilder, CounterVector )
{
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_2 )[0], 2 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_2 )[1], 3 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_5 )[0], 1 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_5 )[1], 1 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_5 )[2], 1 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_5 )[3], 1 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_5 )[4], 1 );
}

TEST_F( TestHuffTreBuilder, CountConstArray )
{
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_3, 5 )[0], 2 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_3, 5 )[1], 3 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_6, 5 )[0], 1 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_6, 5 )[1], 1 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_6, 5 )[2], 1 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_6, 5 )[3], 1 );
    EXPECT_EQ( count_occurance( TEST_BYTE_STREAM_6, 5 )[4], 1 );
}

TEST_F( TestHuffTreBuilder, TryTreeBuilder )
{
    TreeBuilder<decltype(TEST_BYTE_STREAM_1)> builder{};
    builder.populateFreqs( count_occurance( TEST_BYTE_STREAM_1 ) );
    builder.build( true );
    builder.showCodes();
}

TEST_F( TestHuffTreBuilder, TryTreeBuilder2 )
{
    TreeBuilder<decltype(TEST_BYTE_STREAM_4)> builder{};
    builder.populateFreqs( count_occurance( TEST_BYTE_STREAM_4 ) );
    builder.build( true );
    builder.showCodes();
}



int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

