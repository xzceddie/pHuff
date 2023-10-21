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
#include <include/pHuffEncoder.h>
#include <include/utils.h>
#include <array>
#include <cassert>
#include <future>

#define EXPECTED_INFLATE_RATIO_INTER_BUFFER 8
// #define PERF


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

    pHuffSegEncoder( const TreeBuilder<T>& tree_builder)
    : m_treeBuilder( tree_builder )
    {}
    
    void build( const bool eager = true )
    {
        m_treeBuilder.build(eager);
    }

    std::size_t encodeHeader( symbols* out_buf )
    {
        return m_treeBuilder.serialize_huff_codes( out_buf );
    }

    std::size_t encodeContent( symbols* out_buf )
    {
        const auto& orig_buf = *m_origBuf;
        for( const auto sym: orig_buf )
        {
            const auto& this_code = m_treeBuilder.getCodes().at( (int)sym );
            std::copy( this_code.begin(), this_code.end(), std::back_inserter( m_interByteCodes ) );
        }
        // std::cout << "finished inter bytes endoding...\n";
        // std::cout << "interbytes length: " << m_interByteCodes.size() << std::endl;

        return pHuff::utils::pack_buf_avx_256( m_interByteCodes, out_buf );
    }

    std::size_t encodeContentNew( symbols* out_buf )
    {
#ifdef PERF
    auto startTime = std::chrono::high_resolution_clock::now();
#endif

        std::memset( out_buf, 0, m_origBuf->size() );
        const auto& orig_buf = *m_origBuf;
        symbols* curr_ptr = out_buf;
        int curr_bits = 0;

        for( std::size_t ind = 0; ind < orig_buf.size(); ++ind )
        {
            const symbols this_sym = orig_buf[ind];
            const auto this_packed_code = m_treeBuilder.getPakedCodes( this_sym );
            auto& val = *reinterpret_cast<std::uint64_t*>( curr_ptr );
            val |= (this_packed_code >> curr_bits);
            // *(curr_ptr) |= this_packed_code >> (curr_bits + 56);
            // *( reinterpret_cast<std::uint64_t*>(curr_ptr + 1) ) = this_packed_code << curr_bits;

            const std::uint8_t code_size = m_treeBuilder.getCodeLen( this_sym );
            // std::cout << "this_code_size: " << (int)code_size << std::endl;

            auto virtual_bits = curr_bits + code_size;
            curr_bits = ( virtual_bits % 8 );
            curr_ptr += (virtual_bits >> 3);
        }

#ifdef PERF
    auto endTime = std::chrono::high_resolution_clock::now();
    auto dur = endTime - startTime;
    std::cout << "core step used Time ( micro sec ) "
              << std::chrono::duration_cast< std::chrono::microseconds>( dur ).count()
              << std::endl;
#endif
        return curr_ptr - out_buf + ( curr_bits != 0 );
    }

    std::size_t encodeContentOpt( symbols* out_buf )
    {
#ifdef PERF
    auto startTime = std::chrono::high_resolution_clock::now();
#endif

        // -> This is really not needed if we opt for choice 2 of filling the codes
        // std::memset( out_buf, 0, m_origBuf->size() );
        const auto& orig_buf = *m_origBuf;
        symbols* curr_ptr = out_buf;

        symbols this_sym{}, next_sym{}, this_code_size{};
        int curr_bits{}, virtual_bits{};
        std::uint64_t this_packed_code{};

        std::size_t ind = 0;
        while ( ind < orig_buf.size() - 1 )
        {
            this_sym = orig_buf[ind];
            next_sym = orig_buf[ind + 1];
            this_packed_code = m_treeBuilder.getPakedCodesExt( this_sym, next_sym );
            this_code_size = m_treeBuilder.getCodeLenExt( this_sym, next_sym );

            // auto& val = *reinterpret_cast<std::uint64_t*>( curr_ptr );
            // val |= this_packed_code >> curr_bits;

            *(curr_ptr) |= this_packed_code >> (curr_bits + 56);
            *( reinterpret_cast<std::uint64_t*>(curr_ptr + 1) ) = this_packed_code << curr_bits;

            virtual_bits = curr_bits + this_code_size;
            curr_bits = ( virtual_bits & 7 );
            curr_ptr += (virtual_bits >> 3);

            ind += 2;
        }

        if( ind == orig_buf.size() - 1 )
        {
            this_sym = orig_buf[ind];
            this_packed_code = m_treeBuilder.getPakedCodes( this_sym );
            auto& val = *reinterpret_cast<std::uint64_t*>( curr_ptr );
            val |= (this_packed_code >> curr_bits);

            this_code_size = m_treeBuilder.getCodeLen( this_sym );

            virtual_bits = curr_bits + this_code_size;
            curr_bits = ( virtual_bits & 7 );
            curr_ptr += (virtual_bits >> 3);
        }

#ifdef PERF
    auto endTime = std::chrono::high_resolution_clock::now();
    auto dur = endTime - startTime;
    std::cout << "core step used Time ( micro sec ) "
              << std::chrono::duration_cast< std::chrono::microseconds>( dur ).count()
              << std::endl;
#endif
        return curr_ptr - out_buf + ( curr_bits != 0 );
    }

    auto encode( symbols* out_buf )
    {
        const auto header_size = encodeHeader( out_buf );
        out_buf += header_size;
        // std::cout << "finished header encoding...\n";
        // const auto code_size = encodeContent( out_buf );
        // const auto code_size = encodeContentNew( out_buf );
        const auto code_size = encodeContentOpt( out_buf );
        // std::cout << "finished content encoding, code_size: " << code_size << "\n";
        return std::tuple{header_size, code_size, header_size + code_size};
    }
    
};


template < Iterable T >
auto pHuffEncodeSegImp( const std::unique_ptr<pHuffSegEncoder<T>>& encoder, symbols* out_buf)
{
    encoder->build(true);
    // std::cout << "finished tree building...\n";
    return encoder->encode( out_buf );
}

template < Iterable T >
auto pHuffEncodeSeg( const T& in_buf, symbols* out_buf)
{
    std::unique_ptr<pHuffSegEncoder<T>> encoder = std::make_unique<pHuffSegEncoder<T>> (in_buf);
    // std::cout << "finished constructing pHuffSegEncoder...\n";
    return pHuffEncodeSegImp( encoder, out_buf );
}

template < Iterable T >
std::size_t pHuffEncodeSegBody( const TreeBuilder<T>& tree_builder, symbols* out_buf)
{
    std::unique_ptr<pHuffSegEncoder<T>> encoder = std::make_unique<pHuffSegEncoder<T>> (tree_builder);
    return encoder->encodeContent( out_buf );
}


template < Iterable T, std::size_t N_THREAD >
auto pHuffEncodeImp( const T& in_buf, symbols* out_buf)
{
    if constexpr ( N_THREAD == 1)
    {
        const auto [header_size, body_size, enc_size] = pHuffEncodeSeg( in_buf, out_buf );
        return std::pair{header_size, std::array<std::size_t, 1>{ body_size }};
    }

    std::unique_ptr<TreeBuilder<T>> tree_builder = std::make_unique<TreeBuilder<T>> ();
    tree_builder->populateFreqs( count_occurance(in_buf) );
    tree_builder->build(true);
    const auto header_size = tree_builder->serialize_huff_codes(out_buf);
    out_buf += header_size;

    const std::size_t len_seg = in_buf.size() / N_THREAD;
    std::vector< symbols* > out_buf_tmp;
    std::vector< std::future< std::size_t> > fut_vec;
    for( int i=0; i<N_THREAD; ++i )
    {
        symbols* curr_start_out_buf = out_buf + i * ( in_buf.size() / N_THREAD );
        auto start_it = in_buf.begin() + len_seg * i;
        auto end_it  = ( i==N_THREAD - 1 ) ? in_buf.end() : ( start_it + len_seg );
        fut_vec.emplace_back(
            std::async (
                 pHuffEncodeSegBody<std::vector<symbols>>,
                 *tree_builder,
                 curr_start_out_buf
            )
        );
    }

    std::array< std::size_t, N_THREAD > c_sizes;
    for( int i=0; i<fut_vec.size(); ++i)
    {
        fut_vec[i].wait();
        c_sizes[i] = ( fut_vec[i].get() );
    }

    symbols* ptr = out_buf + c_sizes.front();
    for( int th = 1; th < N_THREAD; ++th)
    {
        std::memcpy( ptr, out_buf + th * ( in_buf.size() / N_THREAD ), c_sizes[th] );
        ptr += c_sizes[th];
    }

    return std::pair{header_size, c_sizes};
}

template < Iterable T, std::size_t N_THREAD >
std::size_t pHuffEncode( const T& in_buf, symbols* out_buf)
{
    auto* ptr = reinterpret_cast<MultiThreadHeaderStruct<N_THREAD>*>(out_buf);
    out_buf = reinterpret_cast<symbols*> (ptr+1);

    const auto [header_size, body_sizes] = pHuffEncodeImp< T, N_THREAD> ( in_buf, out_buf );
    return sizeof( MultiThreadHeaderStruct<N_THREAD> )
           + header_size
           + std::accumulate( body_sizes.begin(), body_sizes.end(), 0 );
}



#endif
