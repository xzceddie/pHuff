#include <benchmark/benchmark.h>
#include <gtest/gtest.h>
#include <include/pHuffCounter.h>
#include <include/utils.h>


class TestBitPacking
:public ::testing::Test
{};

static const std::vector<symbols> TEST_BIN_ARRY_1( 10000, 0 );
static symbols TEST_OUT_BUF_NAIIVE_1[10000];
static symbols TEST_OUT_BUF_AVX256_1[10000];

static const std::vector<symbols> TEST_BIN_ARRY_2( 10000, 1 );
static symbols TEST_OUT_BUF_NAIIVE_2[10000];
static symbols TEST_OUT_BUF_AVX256_2[10000];

static const std::vector<symbols> TEST_BIN_ARRY_3 = 
[](){
    std::vector<symbols> local_vec ( 10000, 0 );
    for( int i=0 ;i < local_vec.size(); ++i )
    {
        if( i%2 )
            local_vec.at(i) = 1;
    }
    return local_vec;
}();
static symbols TEST_OUT_BUF_NAIIVE_3[10000];
static symbols TEST_OUT_BUF_AVX256_3[10000];



TEST_F( TestBitPacking, PackSameResult1 )
{ 
    auto res_len_naiive = pack_buf_naiive( TEST_BIN_ARRY_1, TEST_OUT_BUF_NAIIVE_1 );
    auto res_len_avx_256 = pack_buf_naiive( TEST_BIN_ARRY_1, TEST_OUT_BUF_AVX256_1 );
    EXPECT_EQ( res_len_naiive, res_len_avx_256 );
    EXPECT_EQ( memcmp( TEST_OUT_BUF_NAIIVE_1, TEST_OUT_BUF_AVX256_1, res_len_naiive ), 0 );
}
TEST_F( TestBitPacking, PackSameResult2 )
{ 
    auto res_len_naiive = pack_buf_naiive( TEST_BIN_ARRY_2, TEST_OUT_BUF_NAIIVE_2 );
    auto res_len_avx_256 = pack_buf_naiive( TEST_BIN_ARRY_2, TEST_OUT_BUF_AVX256_2 );
    EXPECT_EQ( res_len_naiive, res_len_avx_256 );
    EXPECT_EQ( memcmp( TEST_OUT_BUF_NAIIVE_2, TEST_OUT_BUF_AVX256_2, res_len_naiive ), 0 );
}
TEST_F( TestBitPacking, PackSameResult3 )
{ 
    auto res_len_naiive = pack_buf_naiive( TEST_BIN_ARRY_3, TEST_OUT_BUF_NAIIVE_3 );
    auto res_len_avx_256 = pack_buf_naiive( TEST_BIN_ARRY_3, TEST_OUT_BUF_AVX256_3 );
    EXPECT_EQ( res_len_naiive, res_len_avx_256 );
    EXPECT_EQ( memcmp( TEST_OUT_BUF_NAIIVE_3, TEST_OUT_BUF_AVX256_3, res_len_naiive ), 0 );
}


void bench_naiive_pack_1( benchmark::State& in_state )
{ 
    for( auto _: in_state )
    { 
        pack_buf_naiive( TEST_BIN_ARRY_1, TEST_OUT_BUF_NAIIVE_1 );
    }
}
BENCHMARK( bench_naiive_pack_1 );
void bench_naiive_pack_2( benchmark::State& in_state )
{ 
    for( auto _: in_state )
    { 
        pack_buf_naiive( TEST_BIN_ARRY_2, TEST_OUT_BUF_NAIIVE_2 );
    }
}
BENCHMARK( bench_naiive_pack_2 );
void bench_naiive_pack_3( benchmark::State& in_state )
{ 
    for( auto _: in_state )
    { 
        pack_buf_naiive( TEST_BIN_ARRY_3, TEST_OUT_BUF_NAIIVE_3 );
    }
}
BENCHMARK( bench_naiive_pack_3 );


void bench_avx_256_pack_1( benchmark::State& in_state )
{ 
    for( auto _: in_state )
    { 
        pack_buf_avx_256( TEST_BIN_ARRY_1, TEST_OUT_BUF_AVX256_1 );
    }
}
BENCHMARK( bench_avx_256_pack_1 );
void bench_avx_256_pack_2( benchmark::State& in_state )
{ 
    for( auto _: in_state )
    { 
        pack_buf_avx_256( TEST_BIN_ARRY_2, TEST_OUT_BUF_AVX256_2 );
    }
}
BENCHMARK( bench_avx_256_pack_2 );
void bench_avx_256_pack_3( benchmark::State& in_state )
{ 
    for( auto _: in_state )
    { 
        pack_buf_avx_256( TEST_BIN_ARRY_3, TEST_OUT_BUF_AVX256_3 );
    }
}
BENCHMARK( bench_avx_256_pack_3 );


// BENCHMARK_MAIN();

int main( int argc, char ** argv )
{ 
   ::testing::InitGoogleTest(&argc, argv);
   auto res = RUN_ALL_TESTS();
   ::benchmark::Initialize(&argc, argv);
   ::benchmark::RunSpecifiedBenchmarks();
   return res;
}
