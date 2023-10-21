#ifndef P_HUFF_DECODER_H
#define P_HUFF_DECODER_H

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
#include <include/pHuffEncoder.h>
#include <include/utils.h>
#include <array>
#include <cassert>
#include <future>


using SegHeader = EncSegHeader;
using MaskIntType = std::uint64_t;
using code_type = std::uint64_t;
static constexpr std::size_t P_HUFF_HEADER_SIZE = sizeof( SegHeader );
static constexpr MaskIntType MASK_FIRST_BITS[64]
{
    0x8000000000000000, 0xc000000000000000, 0xe000000000000000, 0xf000000000000000, 0xf800000000000000, 0xfc00000000000000, 0xfe00000000000000, 0xff00000000000000,
    0xff80000000000000, 0xffc0000000000000, 0xffe0000000000000, 0xfff0000000000000, 0xfff8000000000000, 0xfffc000000000000, 0xfffe000000000000, 0xffff000000000000,
    0xffff800000000000, 0xffffc00000000000, 0xffffe00000000000, 0xfffff00000000000, 0xfffff80000000000, 0xfffffc0000000000, 0xfffffe0000000000, 0xffffff0000000000,
    0xffffff8000000000, 0xffffffc000000000, 0xffffffe000000000, 0xfffffff000000000, 0xfffffff800000000, 0xfffffffc00000000, 0xfffffffe00000000, 0xffffffff00000000,
    0xffffffff80000000, 0xffffffffc0000000, 0xffffffffe0000000, 0xfffffffff0000000, 0xfffffffff8000000, 0xfffffffffc000000, 0xfffffffffe000000, 0xffffffffff000000,
    0xffffffffff800000, 0xffffffffffc00000, 0xffffffffffe00000, 0xfffffffffff00000, 0xfffffffffff80000, 0xfffffffffffc0000, 0xfffffffffffe0000, 0xffffffffffff0000,
    0xffffffffffff8000, 0xffffffffffffc000, 0xffffffffffffe000, 0xfffffffffffff000, 0xfffffffffffff800, 0xfffffffffffffc00, 0xfffffffffffffe00, 0xffffffffffffff00,
    0xffffffffffffff80, 0xffffffffffffffc0, 0xffffffffffffffe0, 0xfffffffffffffff0, 0xfffffffffffffff8, 0xfffffffffffffffc, 0xfffffffffffffffe, 0xffffffffffffffff,
};


struct BIT_CODE
{ 
    code_type m_Code;
    unsigned code_len;
    symbols m_Sym;

    __attribute__((always_inline))
    bool match( const code_type in_code ) const
    {
        return m_Code == ( MASK_FIRST_BITS[code_len] & in_code );
    }
};

struct BIT_CODE_EXT
{
    BIT_CODE m_PrimaryCode;
    unsigned m_extraCnt;
    BIT_CODE* others{ nullptr };

    BIT_CODE_EXT() = default;
    BIT_CODE_EXT( const BIT_CODE in_code )
    : m_PrimaryCode{ in_code }
    {}
    ~BIT_CODE_EXT() { if( !others ) delete[] others; }
    
    void push ( const BIT_CODE in_code )
    {
        if ( !m_PrimaryCode.code_len )
        {
            m_PrimaryCode = in_code;
        }
        else
        {
            if( !others )
                others = new BIT_CODE[256];
            others[ m_extraCnt++ ] = in_code;
        }
    }
    
    auto decode( const code_type in_code ) const
    {
        if ( m_PrimaryCode.match( in_code ) )
        {
            return std::pair{m_PrimaryCode.m_Sym, m_PrimaryCode.code_len};
        }
        for( int i=0; i < m_extraCnt; ++i )
        {
            if( others[i].match( in_code ) )
            {
                return std::pair{others[i].m_Sym, others[i].code_len};
            }
        }
        throw std::runtime_error( "Decode Fail!!!\n") ;
    }
};


__attribute__((always_inline))
void advance_bits( symbols*& start_byte, std::uint8_t& curr_bits, const std::uint8_t load_bit_count )
{
    auto virtual_bits = curr_bits + load_bit_count;
    curr_bits = ( virtual_bits % 8 );
    start_byte += (virtual_bits >> 3);
}

__attribute__((always_inline))
code_type load_bits_impl( symbols*& start_byte, std::uint8_t& curr_bits, const std::uint8_t load_bit_count )
{
    std::uint64_t tmp_code = 0;
    tmp_code = MASK_FIRST_BITS[ load_bit_count ] & (
            ( *(reinterpret_cast<std::uint64_t*>( start_byte )) << curr_bits ) |
            (*(reinterpret_cast<std::uint64_t*>(start_byte) + 1) >> ( 64 - curr_bits ))
            );
    return tmp_code;
}


template <bool advance>
__attribute__((always_inline))
code_type load_bits( symbols*& start_byte, std::uint8_t& curr_bits, const std::uint8_t load_bit_count )
{}

template <>
__attribute__((always_inline))
code_type load_bits<true> ( symbols*& start_byte, std::uint8_t& curr_bits, const std::uint8_t load_bit_count )
{
    const auto tmp_code = load_bits_impl( start_byte, curr_bits, load_bit_count );
    advance_bits( start_byte, curr_bits, load_bit_count );
    return tmp_code;

}
template <>
__attribute__((always_inline))
code_type load_bits<false> ( symbols*& start_byte, std::uint8_t& curr_bits, const std::uint8_t load_bit_count )
{
    const auto tmp_code = load_bits_impl( start_byte, curr_bits, load_bit_count );
    return tmp_code;
}


template < size_t LUT_LEN >
struct pHuffSegDecoder
{
private:
    void* m_pCompBuff;
    void* m_pCompContent;
    std::size_t m_compLen;
    std::vector<BIT_CODE> m_Codes;
    std::array<BIT_CODE_EXT, LUT_LEN> m_Lut;

    const std::vector<BIT_CODE>
    __unpackCodes( symbols* packed_codes_ptr, const SegHeader& phuff_seg_header ) const
    {
        std::vector<BIT_CODE> codes;
        codes.reserve( 256 );
        symbols* curr_ptr{ packed_codes_ptr };
        std::uint8_t curr_bits{};
        for( std::size_t i = 0; i < 256; ++i )
        {
            const std::size_t tmp_code_len = phuff_seg_header.m_CodeLens[i];
            const code_type tmp_code = load_bits<true>( curr_ptr, curr_bits, tmp_code_len );
            codes.push_back( BIT_CODE{tmp_code, static_cast<unsigned>(tmp_code_len), static_cast<symbols>(i)} );
        }
        return codes;
    }


    void __buildLUT()
    {
        for( const auto bit_code: m_Codes )
        {
            if( bit_code.code_len >= LUT_LEN )
            {
                auto ind = bit_code.m_Code >> ( sizeof( code_type ) - LUT_LEN );
                m_Lut[ ind ].push(bit_code);
            }
            else
            {
                const auto start_ind = bit_code.m_Code >> ( sizeof( code_type ) - LUT_LEN );
                const auto end_ind = start_ind + ( 2 << (LUT_LEN - bit_code.code_len) ) - 1;
                for( auto ind = start_ind; ind < end_ind; ++ind )
                {
                    m_Lut[ind].push( bit_code );
                }
            }
            
        }
    }

public:
    pHuffSegDecoder( void* comp_buff, const std::size_t comp_len )
    : m_pCompBuff{ comp_buff }
    , m_compLen{ comp_len }
    {}

    void decodeHeader()
    {
        const SegHeader& hdr = *( reinterpret_cast<SegHeader*>( m_pCompBuff ) );
        symbols* packed_codes = reinterpret_cast<symbols*>(m_pCompBuff) + P_HUFF_HEADER_SIZE;
        m_Codes = __unpackCodes( packed_codes, hdr );
        const auto sum_code_len = std::accumulate( std::begin( hdr.m_CodeLens ), std::end( hdr.m_CodeLens ), 0 );
        m_pCompContent = packed_codes + ( sum_code_len%8 ? 1 + ( sum_code_len >> 3 ) : (sum_code_len >> 3) );
    }


    std::size_t decodeContent( symbols* out )
    {
        std::size_t dec_len{};
        symbols* curr_ptr = m_pCompContent;
        std::uint8_t curr_bits{};
        while( curr_ptr <= m_pCompBuff + m_compLen )
        {
            const code_type this_code = load_bits<false>( curr_ptr, curr_bits, 64 );
            const auto& bit_code_ext = m_Lut[this_code >> ( 64 - LUT_LEN )];
            const auto[ sym, this_code_len ] = bit_code_ext.decode( this_code );
            advance_bits( curr_ptr, curr_bits, this_code_len );
            *out++ = sym;
            dec_len++;
        }
        return dec_len;
    }
    
    std::size_t decode( symbols* out )
    {
        decodeHeader();
        const auto res_len = decodeContent( out );
        return res_len;
    }

};



#endif
