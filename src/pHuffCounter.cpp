#ifndef P_HUFF_COUNTER_H
#define P_HUFF_COUNTER_H

#include <iostream>
#include <algorithm>
#include <numeric>
#include <vector>
#include <ranges>
#include <memory>
#include <variant>
#include <string_view>
#include <fstream>
#include <array>
#include <cstddef>


#define CNT_ALL_ASCII 256


std::array<std::size_t, CNT_ALL_ASCII>
count_occurance( const uint8_t* in_buf, std::size_t in_len )
{
    std::array<std::size_t, CNT_ALL_ASCII> res_list{};
    for( std::size_t p=0; p < in_len; ++p )
    {
        res_list.at(in_buf[p])++;
    }
    return res_list;
}


template< class T >
concept Iterable = requires(T&& t) {
  std::begin(std::forward<T>(t));
  std::end  (std::forward<T>(t));
};


template < Iterable T >
std::array<std::size_t, CNT_ALL_ASCII>
count_occurance( const T& in_buf )
{
    std::array<std::size_t, CNT_ALL_ASCII> res_list{};
    for( const auto& ele: in_buf )
    {
        res_list.at( ele )++;
    }
    return res_list;
}



#endif
