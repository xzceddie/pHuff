#include <include/pHuffEncoder.h>
#include <cstdint>
#include <memory>
#include <thread>
#include <future>

template < Iterable T >
std::size_t pHuffEncodeSegImp( const std::unique_ptr<pHuffSegEncoder<T>>& encoder, symbols* out_buf)
{
    encoder->build(true);
    encoder->encode( out_buf );
}

template < Iterable T >
std::size_t pHuffEncodeSeg( const T& in_buf, symbols* out_buf)
{
    std::unique_ptr<pHuffSegEncoder<T>> encoder = std::make_unique<pHuffSegEncoder<T>> (in_buf);
    pHuffEncodeSegImp( encoder, out_buf );
}


template < Iterable T >
std::size_t pHuffEncode( const T& in_buf, symbols* out_buf, const std::size_t n_thread = 1)
{
    std::unique_ptr<pHuffSegEncoder<T>> encoder = std::make_unique<pHuffSegEncoder<T>> (in_buf);
    if (n_thread == 1)
        return pHuffEncodeSegImp( std::move(encoder), out_buf );

    const std::size_t len_seg = in_buf.size() / n_thread;
    std::vector< symbols* > out_buf_tmp;
    std::vector< std::future< std::size_t> > fut_vec;
    for( int i=0; i<n_thread; ++i )
    {
        symbols* curr_start_out_buf = out_buf + i * ( in_buf.size() / n_thread );
        auto start_it = in_buf.begin() + len_seg * i;
        auto end_it  = ( i==n_thread - 1 ) ? in_buf.end() : ( start_it + len_seg );
        fut_vec.emplace_back(
            std::async (
                 pHuffEncodeSegImp<std::vector<symbols>>,
                 encoder,
                 curr_start_out_buf
            )
        );
    }

    std::vector< std::size_t > c_sizes;
    for( auto& t: fut_vec )
    {
        t.wait();
        c_sizes.push_back( t.get() );
    }
}
