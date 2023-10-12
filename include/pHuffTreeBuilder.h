#ifndef P_HUFF_TREE_BUILDER_H
#define P_HUFF_TREE_BUILDER_H

#include <iostream>
#include <algorithm>
#include <numeric>
#include <vector>
#include <ranges>
#include <memory>
#include <variant>
#include <string_view>
#include <include/pHuffCounter.h>
#include <include/utils.h>
#include <array>
#include <cassert>

#define assertm(exp, msg) assert(((void)msg, exp))


using symbols = std::uint8_t;

struct SymCnt
{
public:
    std::size_t m_Cnt;
    std::vector<symbols> m_Sym;
    SymCnt()
    {
        m_Sym.reserve( CNT_ALL_ASCII );
    }

    SymCnt( const symbols sym, const std::size_t s )
    : SymCnt()
    {
      m_Cnt = s;
      m_Sym.push_back( sym );
    }

    void Absorb( const SymCnt& another )
    {
        m_Cnt+=another.m_Cnt;
        std::copy( another.m_Sym.begin(), another.m_Sym.end(), std::back_inserter( m_Sym ) );
    }
};

struct FreqLess
{
    bool operator()( const SymCnt& sym_cnt_1, const SymCnt& sym_cnt_2 )
    {
        if ( sym_cnt_1.m_Cnt > sym_cnt_2.m_Cnt )
        {
            return true;
        }
        return false;
    }
};


template < Iterable T >
struct TreeBuilder
{
private:
    std::vector< SymCnt > m_FoundSyms{}; // building this up, it will be maintained as a min heap
    std::vector< std::vector< symbols > > m_Codes{};
    bool m_CodesReversed{false};

    // false: build not finished
    // tree: build finished
    bool __buildOneStep()
    {  
        assertm ( !m_FoundSyms.empty(), "Empty Symbols Found to Build Tree! Abort\n" );
        if( m_FoundSyms.size() == 1 )
            return true;
        
        // std::cout << "Building... \n";
        // for(auto& ele: m_FoundSyms)
        // {
        //     std::cout << "count: " << ele.m_Cnt << ", symbols: ";
        //     for(auto& sym: ele.m_Sym)
        //         std::cout << (int)sym <<" ";
        //     std::cout << std::endl;
        // }
        // std::cout << std::endl;

        std::pop_heap( m_FoundSyms.begin(), m_FoundSyms.end(), FreqLess{} );
        std::pop_heap( m_FoundSyms.begin(), m_FoundSyms.end() - 1, FreqLess{} );

        auto& min = m_FoundSyms.back();
        auto& sec_min = *( m_FoundSyms.end() - 2 );
        
        for( const symbols& ele: sec_min.m_Sym )
        {
            m_Codes.at( (int)ele ).push_back( 1 );
            // std::cout << "Code at: [" << (int)ele << "]: ";
            // for(auto ele_ele: m_Codes.at((int)ele))
            //     std::cout << (int)ele_ele << " ";
            // std::cout << std::endl;
        }
        for( const symbols& ele: min.m_Sym )
        {
            m_Codes.at( (int)ele ).push_back( 0 );
            // std::cout << "Code at [" << (int)ele << "]: ";
            // for(auto ele_ele: m_Codes.at((int)ele))
            //     std::cout << (int)ele_ele << " ";
            // std::cout << std::endl;
        }
        // std::cout << std::endl;

        sec_min.Absorb( min );

        m_FoundSyms.pop_back();
        std::push_heap( m_FoundSyms.begin(), m_FoundSyms.end(), FreqLess{} );

        return false;
    }

public:
    TreeBuilder()
        :m_Codes( CNT_ALL_ASCII )
    {
        m_FoundSyms.reserve(CNT_ALL_ASCII);
        for( auto & vec:m_Codes )
        {
            /*In the worst case, the number of BITS in an encoded 
             * symbol is approx. the same as the count the possible symbols
             */
            vec.reserve( CNT_ALL_ASCII );
        }
    }

    void populateFreqs( const std::array< std::size_t, CNT_ALL_ASCII >& freqs )
    {
        for ( auto it = freqs.begin(); it != freqs.end(); ++it )
        {
            if (*it > 0 )
            {
                auto curr_sym = it - freqs.begin();
                m_FoundSyms.push_back( SymCnt{ static_cast<symbols>(curr_sym), *it } );
            }
        }
        
        std::make_heap( m_FoundSyms.begin(), m_FoundSyms.end(), FreqLess{} );
    }
    
    /*
     * If eager, after building the tree, the codes will be reversed
     * */
    void build( const bool eager = true )
    {
        while( !__buildOneStep() )
        {}
        
        if( !eager )
        {
            m_CodesReversed = false;
            return;
        }
        
        /*
         * The Codes needed to be reversed
         */
        std::for_each( 
            m_Codes.begin(), m_Codes.end(),
            [this]( auto& vec ){
                if (!vec.empty())
                {
                    const std::size_t vec_len = vec.size();
                    for( int i=0; i < vec_len / 2; ++i )
                    {
                        std::swap( vec[i], vec[vec_len - 1 - i] );
                    }
                }
            }
        );
        m_CodesReversed = true;
        return;
    }

    /*
     * Meant for Debug: Always print correct codes no matter the value of m_CodesReversed
     **/
    void showCodes()
    { 
        std::cout << "-- Printing the Generated Codes ... ...\n";
        for( int i=0; i < m_Codes.size(); ++i )
        { 
            const auto& code_vec = m_Codes[i];
            if( !code_vec.empty() )
            { 
                std::cout << "Symbol: " << i <<  " Code: ";
                if (m_CodesReversed)
                {
                    for( auto code: code_vec )
                    { if( code == 0 ) std::cout << "0 "; else if ( code == 1 ) std::cout << "1 "; }
                    std::cout << "\n";
                }
                else
                {
                    for( auto it = code_vec.rbegin(); it != code_vec.rend(); ++it )
                    { auto code = *it; if( code == 0 ) std::cout << "0 "; else if ( code == 1 ) std::cout << "1 "; }
                    std::cout << "\n";
                }
            }
        }
    }

    //

    #pragma pack(1)
    struct EncodedHeader
    {
        std::uint16_t m_CntCodes;
        std::uint16_t m_CodeLength;
    };
    #pragma pack()

    // very naiive serialization
    std::size_t serialize_huff_codes(void * out_buf)
    {
        std::vector< symbols > concat_codes;
        for( const auto & ele: getCodes() )
        {
            std::copy( ele.begin(), ele.end(), std::back_inserter( concat_codes ) );
        }

        const auto [ code_cnt, code_len] = [concat_codes, this](){
            return getCodes().size(), concat_codes.size() % 8 ?  1 + (concat_codes.size()>>3) : concat_codes.size()>>3;
        }();

        EncodedHeader* out_ptr = static_cast<EncodedHeader*> (out_buf);
        out_ptr->m_CntCodes = code_cnt;
        out_ptr->m_CodeLength = code_len;

        auto packed_size = pHuff::utils::pack_buf_avx_256(
            concat_codes, static_cast<symbols*>(out_buf) + sizeof(EncodedHeader)
        );
        assertm(packed_size == code_len, "Oops! serialization of your Huffman code probably goes wrong\n");
    }
    

    std::vector<std::vector<symbols>> getCodes() const
    {
        return m_Codes;
    }
    
};

#endif
