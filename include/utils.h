#ifndef UTILS_H
#define UTILS_H


#include <iostream>
#include <array>
#include <algorithm>
#include <include/pHuffCounter.h>
#include <immintrin.h>

typedef std::uint8_t symbols;
namespace pHuff::utils {
#define BYTE uint8_t

template <size_t AlphaBetaSize, size_t ThreadSize>
void binFreq( const std::array<const size_t[AlphaBetaSize], ThreadSize>& inFreqTbls,
              size_t (&combined)[AlphaBetaSize])
{
    // std::fill( std::begin( combined ), std::begin( combined ) + AlphaBetaSize, 0 );
    for( int i = 0; i < ThreadSize; ++i )
    {
        for ( int j = 0 ; j < AlphaBetaSize; ++j )
        {
            combined[j] += inFreqTbls[i][j];
        }
    }
}



template <size_t AlphaBetaSize>
void buildFreqTbl( const void * inSeq, const size_t inSize, size_t (&tbl)[AlphaBetaSize] )
{
    const BYTE* src = static_cast<const uint8_t*>(inSeq);
    for ( size_t i = 0; i < inSize; ++i )
        tbl[ src[ i ] ]++;
}


/*
 * ****************************************************************************
 */

template <std::size_t PACK_UNIT, Iterable T>
auto preprocess( const T& in_buf )
{
    const auto rounds = in_buf.size() % PACK_UNIT ? in_buf.size() / PACK_UNIT + 1 : in_buf.size() / PACK_UNIT;
    std::vector<symbols> modified_in{ in_buf.begin(), in_buf.end() };
    if (!in_buf.size() % PACK_UNIT)
    {
        for( int i=0; i <  ( PACK_UNIT -in_buf.size() % PACK_UNIT ); ++i )
        {
            modified_in.push_back( (symbols)0 );
        }
    }
    return rounds;
}


template < Iterable T >
std::size_t pack_buf_naiive( const T& in_buf, symbols* out_buf )
{
    const auto rounds = preprocess<8, T>( in_buf );

    for( std::size_t r = 0; r < rounds; ++r )
    {
        *out_buf++ = 
            in_buf[ r << 3 ] << 7
          + in_buf[ 1 + (r << 3) ] << 6
          + in_buf[ 2 + (r << 3) ] << 5
          + in_buf[ 3 + (r << 3) ] << 4
          + in_buf[ 4 + (r << 3) ] << 3
          + in_buf[ 5 + (r << 3) ] << 2
          + in_buf[ 6 + (r << 3) ] << 1
          + in_buf[ 7 + (r << 3) ]
          ;
    }

    return rounds;
}


inline uint32_t avs_256_pack_32_bytes(const void* array) {
    __m256i tmp = _mm256_loadu_si256((const __m256i *) array);
    tmp = _mm256_cmpgt_epi8(tmp, _mm256_setzero_si256());
    return _mm256_movemask_epi8(tmp);
}

template < Iterable T >
std::size_t pack_buf_avx_256( const T& in_buf, symbols* out_buf )
{
    std::uint32_t* out_buf_cpy = reinterpret_cast<std::uint32_t*>( out_buf );
    const auto rounds = preprocess<32, T>( in_buf ); // this is rounds in 4 bytes, not 1
    const auto final_rounds = in_buf.size() % sizeof(symbols) ? in_buf.size() / sizeof(symbols) + 1 : in_buf.size() / sizeof(symbols);
    for( std::size_t i = 0; i < rounds; ++i )
    {
        *out_buf_cpy++ = avs_256_pack_32_bytes( in_buf.data() + 32 * i );
    }
    return final_rounds;
}

}// namespace pHuff::utils



#endif

