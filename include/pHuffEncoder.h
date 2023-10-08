#ifndef P_HUFF_ENCODER_H
#define P_HUFF_ENCODER_H

#include <iostream>
#include <algorithm>
#include <numeric>
#include <vector>
#include <ranges>
#include <memory>
#include <variant>
#include <string_view>
#include <include/pHuffCounter.h>
#include <include/pHuffTreeBuilder.h>
#include <include/utils.h>
#include <array>
#include <cassert>


#define EXPECTED_INFLATE_RATIO_INTER_BUFFER 8


// template < Iterable T >
// struct pHuffSegEncoder
// {
// private:
//     TreeBuilder<T> m_treeBuilder{};
// public:
//     pHuffSegEncoder( const std::array< std::size_t, CNT_ALL_ASCII >& freqs )
//     {
//         m_treeBuilder.populateFreqs( freqs );
//         m_treeBuilder.build(true);
//     }
// 
// 
//     std::size_t encode( const Iterable<T>& in_buf, const symbols* out_buf )
//     {
//         for( const auto sym: in_buf )
//         {
//             auto& this_code = m_treeBuilder.getCodes().at( sym );
//         }
//         return 0;
//     }
// };

template < Iterable T >
struct pHuffSegEncoder
{
private:
    std::array< std::size_t, CNT_ALL_ASCII> m_freqs;
    TreeBuilder<T> m_treeBuilder{};
    std::vector< symbols > m_interByteCodes{};
    const T* m_origBuf;
public:
    pHuffSegEncoder( const T& in_buf )
    : m_freqs{ count_occurance( in_buf ) }
    , m_origBuf( &in_buf )
    {
        m_treeBuilder.populateFreqs( m_freqs );
        m_interByteCodes.reserve( in_buf.size() * EXPECTED_INFLATE_RATIO_INTER_BUFFER );
    }
    
    void build( const bool eager = true )
    {
        m_treeBuilder.build(eager);
    }

    std::size_t encode( symbols* out_buf, symbols* out_buf )
    {
        const auto& orig_buf = *m_origBuf;
        for( const auto sym: orig_buf )
        {
            const auto& this_code = m_treeBuilder.getCodes().at( sym );
            std::copy( this_code.begin(), this_code.end(), std::back_inserter( m_interByteCodes ) );
        }

        return pHuff::utils.pack_buf_avx_256( m_interByteCodes, out_buf );
    }
    
};




#endif
