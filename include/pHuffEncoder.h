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
#include <array>
#include <cassert>



template < Iterable T >
struct pHuffSegEncoder
{
private:
    TreeBuilder<T> m_treeBuilder{};
public:
    pHuffSegEncoder( const std::array< std::size_t, CNT_ALL_ASCII >& freqs )
    {
        m_treeBuilder.populateFreqs( freqs );
        m_treeBuilder.build(true);
    }


    std::size_t encode( const Iterable<T>& in_buf, const symbols* out_buf )
    {
        for( const auto sym: in_buf )
        {
            auto& this_code = m_treeBuilder.getCodes().at( sym );
        }
        return 0;
    }
};




#endif
