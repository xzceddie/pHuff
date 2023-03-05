#ifndef UTILS_H
#define UTILS_H


#include <iostream>
#include <array>
#include <algorithm>

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


#endif

