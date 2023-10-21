#include <iostream>
#include <sys/wait.h>
#include <include/pHuffEncoder.h>
#include <include/pHuffDecoder.h>
#include <vector>
#include <fstream>
#include <chrono>

#define LUT_LEN 10

int main( int argc, const char** argv )
{
    if( argv[1][0] == 'c')
    {
        std::cout << "doing compression\n";
    }

    std::cout << "got input file: " << argv[2] << std::endl;

    std::ifstream file(argv[2], std::ios::binary); 
    std::vector< symbols > in_buf{std::istreambuf_iterator<char>{file}, {}};

    symbols * out_buf = new symbols[in_buf.size() * 10];

    auto startTime = std::chrono::high_resolution_clock::now();
    auto cmp_size = pHuffEncode<std::vector<symbols>, 1>( in_buf, out_buf );
    auto endTime = std::chrono::high_resolution_clock::now();
    auto dur = endTime - startTime;
    std::cout << "used Time ( micro sec ) "
              << std::chrono::duration_cast< std::chrono::microseconds>( dur ).count()
              << std::endl;

    std::cout << "compressed size: " << cmp_size << std::endl;

    return 0;
}
