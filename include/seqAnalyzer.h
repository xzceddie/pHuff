#ifndef SEQ_ANALYZER_H
#define SEQ_ANALYZER_H


#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <thread>
#include <utils.h>


#define BYTE_NUM 256


constexpr bool is_pow_2( const size_t input )
{
    return input > 1 ? 
                        (input%2 ? false : is_pow_2( input/2 )) :
                        false;
}


template <size_t AlphaBetaSize,
          size_t ThreadSize = 1,
          typename T = typename std::enable_if<is_pow_2(AlphaBetaSize)>::type>
class SeqAnalyzer
{
public:
    SeqAnalyzer( const void * inSeq, size_t inSize )
    : m_seqSize( inSize )
    {
        const BYTE* src = static_cast<const uint8_t*>(inSeq);
        const size_t len_each_seg = inSize / ThreadSize;
        std::thread tmp_threads[ ThreadSize ];
        for ( int i = 0; i < ThreadSize; ++i )
        {
            const BYTE* this_ptr = src + len_each_seg * i;
            const size_t this_len = (i == ThreadSize ? src + inSize - this_ptr : len_each_seg );
            tmp_threads[i] = std::thread( buildFreqTbl<AlphaBetaSize>, this_ptr, this_len, m_freqTbls[i]);
        }
        
        for ( int i = 0; i < ThreadSize; ++i )
        {
            tmp_threads[i].join();
        }
    }

    template <size_t inSize>
    SeqAnalyzer( const char (&input) [inSize] )
    : m_seqSize( inSize )
    {
        SeqAnalyzer( input, inSize );
    }

private:
    size_t m_cntByte[AlphaBetaSize] = {};
    size_t m_seqSize;
    std::array<size_t[AlphaBetaSize], ThreadSize> m_freqTbls = {};
};




#endif
