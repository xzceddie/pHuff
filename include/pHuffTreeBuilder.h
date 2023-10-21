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

#pragma pack(1)
struct EncSegHeader
{
    // std::uint16_t m_CntCodes;
    // std::uint16_t m_CodeLength;
    std::uint8_t m_CodeLens[CNT_ALL_ASCII];
};
template< std::size_t N_THREAD >
struct MultiThreadHeaderStruct
{
    std::uint8_t n_thread = N_THREAD;
    std::uint16_t encoded_header_size;
    std::size_t encoded_body_sizes[N_THREAD];
};
#pragma pack()

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


static constexpr std::uint64_t CODE_PACK_MASK_64[64] = 
{
    0x8000000000000000, 0x4000000000000000, 0x2000000000000000, 0x1000000000000000,
    0x0800000000000000, 0x0400000000000000, 0x0200000000000000, 0x0100000000000000,
    0x0080000000000000, 0x0040000000000000, 0x0020000000000000, 0x0010000000000000,
    0x0008000000000000, 0x0004000000000000, 0x0001ffffffffffff, 0x0001000000000000,
    0x0000800000000000, 0x0000400000000000, 0x0000200000000000, 0x0000100000000000,
    0x0000080000000000, 0x0000040000000000, 0x0000020000000000, 0x0000010000000000,
    0x0000008000000000, 0x0000004000000000, 0x0000002000000000, 0x0000001000000000,
    0x0000000800000000, 0x0000000400000000, 0x0000000200000000, 0x0000000100000000,
    0x0000000080000000, 0x0000000040000000, 0x0000000020000000, 0x0000000010000000,
    0x0000000008000000, 0x0000000004000000, 0x0000000002000000, 0x0000000001000000,
    0x0000000000800000, 0x0000000000400000, 0x0000000000200000, 0x0000000000100000,
    0x0000000000080000, 0x0000000000040000, 0x0000000000020000, 0x0000000000010000,
    0x0000000000008000, 0x0000000000004000, 0x0000000000002000, 0x0000000000001000,
    0x0000000000000800, 0x0000000000000400, 0x0000000000000200, 0x0000000000000100,
    0x0000000000000080, 0x0000000000000040, 0x0000000000000020, 0x0000000000000010,
    0x0000000000000008, 0x0000000000000004, 0x0000000000000002, 0x0000000000000001,
};




template < Iterable T >
struct TreeBuilder
{
private:
    std::uint64_t m_PackedCodesExt[CNT_ALL_ASCII][CNT_ALL_ASCII]{};
    std::uint8_t m_CodeLensExt[CNT_ALL_ASCII][CNT_ALL_ASCII]{};

    std::vector< SymCnt > m_FoundSyms{}; // building this up, it will be maintained as a min heap
    std::vector< std::vector< symbols > > m_Codes{};
    std::vector< std::uint64_t > m_PackedCodes{};
    std::vector< std::uint8_t > m_CodeLens{};

    bool m_CodesReversed{false};
    bool m_Built{false};

    // false: build not finished
    // tree: build finished
    bool __buildOneStep()
    {  
        assertm ( !m_FoundSyms.empty(), "Empty Symbols Found to Build Tree! Abort\n" );
        if( m_FoundSyms.size() == 1 )
            return true;
        
        std::pop_heap( m_FoundSyms.begin(), m_FoundSyms.end(), FreqLess{} );
        std::pop_heap( m_FoundSyms.begin(), m_FoundSyms.end() - 1, FreqLess{} );

        auto& min = m_FoundSyms.back();
        auto& sec_min = *( m_FoundSyms.end() - 2 );
        
        for( const symbols& ele: sec_min.m_Sym )
        {
            m_Codes.at( (int)ele ).push_back( 1 );
        }
        for( const symbols& ele: min.m_Sym )
        {
            m_Codes.at( (int)ele ).push_back( 0 );
        }

        sec_min.Absorb( min );

        m_FoundSyms.pop_back();
        std::push_heap( m_FoundSyms.begin(), m_FoundSyms.end(), FreqLess{} );

        return false;
    }

    void __packCodes()
    {
        for (const auto& codes: m_Codes)
        {
            assert( codes.size() <= 64 );
            std::uint64_t tmp_code{};
            for( std::size_t ind = 0; ind < codes.size(); ++ind )
            {
                tmp_code &= ( codes[ ind ] * CODE_PACK_MASK_64[ ind ] );
            }

            m_PackedCodes.push_back( tmp_code );
            m_CodeLens.push_back( codes.size() );
        }
    }

    void __buildExtendedCodes()
    {
        for( int first_sym = 0; first_sym < CNT_ALL_ASCII; ++first_sym )
        {
            for( int second_sym = 0; second_sym < CNT_ALL_ASCII; ++second_sym )
            {
                auto& code = m_PackedCodesExt[ first_sym ][ second_sym ] ;
                code = m_PackedCodes[ first_sym ] + ( m_PackedCodes[ second_sym ] >> m_CodeLens[ first_sym ] );
                m_CodeLensExt[ first_sym ][ second_sym ] = m_CodeLens[ first_sym ] + m_CodeLens[ second_sym ];
            }
        }
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

        m_Built = true;
        __packCodes();
        __buildExtendedCodes();
        
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

    // very naiive serialization
    std::size_t serialize_huff_codes(void * out_buf)
    {
        std::vector< symbols > concat_codes;
        for( const auto & ele: getCodes() )
        {
            std::copy( ele.begin(), ele.end(), std::back_inserter( concat_codes ) );
        }


        const auto [ code_cnt, code_len] = [concat_codes, this](){
            return std::pair{getCodes().size(),
                             concat_codes.size() % 8 ?
                                1 + (concat_codes.size()>>3) : concat_codes.size()>>3};
        }();

        EncSegHeader* out_ptr = static_cast<EncSegHeader*> (out_buf);
        // out_ptr->m_CntCodes = code_cnt;
        // out_ptr->m_CodeLength = code_len;
        std::copy( m_CodeLens.begin(), m_CodeLens.end(), std::begin(out_ptr->m_CodeLens) );

        auto packed_size = pHuff::utils::pack_buf_avx_256(
            concat_codes, static_cast<symbols*>(out_buf) + sizeof(EncSegHeader)
        );
        assertm(packed_size == code_len, "Oops! serialization of your Huffman code probably goes wrong\n");
        return packed_size;
    }
    

    const std::vector<std::vector<symbols>>& getCodes() const &
    {
        return m_Codes;
    }
    
    std::vector<std::vector<symbols>>& getCodes() &
    {
        return m_Codes;
    }

    const std::vector<std::uint64_t>& getPakedCodes() const &
    {
        return m_PackedCodes;
    }
    
    std::vector<std::uint64_t>& getPakedCodes() &
    {
        return m_PackedCodes;
    }

    std::uint64_t getPakedCodes( const int sym ) const
    {
        return m_PackedCodes[ sym ];
    }

    std::uint8_t getCodeLen( const int in_sym ) const
    {
        return m_CodeLens[ in_sym ];
    }

    std::uint64_t getPakedCodesExt( const int first_sym, const int second_sym ) const
    {
        return m_PackedCodesExt[ first_sym ][ second_sym ];
    }

    std::uint8_t getCodeLenExt( const int first_sym, const int second_sym ) const
    {
        return m_CodeLensExt[ first_sym ][ second_sym ];
    }
};

#endif
