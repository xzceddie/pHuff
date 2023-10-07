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

static const std::vector<std::uint8_t> TEST_BYTE_STREAM_7 { 0, 1, 2, 3, 4, 4, 4, 5, 5, 6, 6, 6, 6, 6, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 10, 10, 10, 10, 11, 12, 13 };

static const std::vector< std::vector< symbols> > TEST_IS_NOT_PREFIX{
    {0, 1},
    {1, 1},
    {1, 0, 1},
    {0, 1, 0, 0},
};
static const std::vector< std::vector< symbols> > TEST_IS_PREFIX{
    {1, 1},
    {1, 0, 1},
    {1, 0, 0, 1},
    {0, 1, 0, 0},
};

bool helper_is_prefix_code( const std::vector< std::vector< symbols > >& in_codes )
{
    for(std::size_t i = 0; i < in_codes.size(); ++i)
    {
        if( !in_codes.at(i).empty() )
        {
            for(std::size_t j = 0; j < in_codes.size(); ++j)
            {
                if( j != i &&
                        in_codes.at(i).size() <= in_codes.at(j).size() && 
                        ( !memcmp( in_codes.at(i).data(), in_codes.at(j).data(), in_codes.at(i).size()*sizeof(symbols)) ) )
                {
                    std::cout << "PREFIX CODE BREACHES ! \n"
                        << "CODE1: ";
                    for( auto& ele: in_codes.at(i) )
                        std::cout << (int)ele << " ";
                    std::cout << std::endl;
                    std::cout << "CODE2: ";
                    for( auto& ele: in_codes.at(j) )
                        std::cout << (int)ele << " ";
                    std::cout << std::endl;
                    return false;
                }

            }
        }
    }
    return true;
}



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

TEST_F( TestHuffTreBuilder, PrefexDetector )
{
    EXPECT_EQ( helper_is_prefix_code( TEST_IS_NOT_PREFIX ), false );
    EXPECT_EQ( helper_is_prefix_code( TEST_IS_PREFIX ), true );
}

TEST_F( TestHuffTreBuilder, TryTreeBuilder )
{
    TreeBuilder<decltype(TEST_BYTE_STREAM_1)> builder{};
    builder.populateFreqs( count_occurance( TEST_BYTE_STREAM_1 ) );
    builder.build( true );
    builder.showCodes();
    EXPECT_EQ( helper_is_prefix_code( builder.getCodes() ), true );
}

TEST_F( TestHuffTreBuilder, TryTreeBuilder2 )
{
    TreeBuilder<decltype(TEST_BYTE_STREAM_4)> builder{};
    builder.populateFreqs( count_occurance( TEST_BYTE_STREAM_4 ) );
    builder.build( true );
    builder.showCodes();
    EXPECT_EQ( helper_is_prefix_code( builder.getCodes() ), true );
}

TEST_F( TestHuffTreBuilder, TryTreeBuilder3 )
{
    TreeBuilder<decltype(TEST_BYTE_STREAM_7)> builder{};
    builder.populateFreqs( count_occurance( TEST_BYTE_STREAM_7 ) );
    builder.build( true );
    builder.showCodes();
    EXPECT_EQ( helper_is_prefix_code( builder.getCodes() ), true );
}



int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

