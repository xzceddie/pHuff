#include <iostream>
#include <sys/wait.h>
#include <include/pHuffEncoder.h>
#include <vector>
#include <fstream>


int main( int argc, const char** argv )
{
    if( argv[1][0] == 'c')
    {
        std::cout << "doing compression\n";
    }

    std::cout << "input file: " << argv[2];

    std::ifstream file(argv[2], std::ios::binary); 
    std::vector< symbols > in_buf{std::istreambuf_iterator<char>{file}, {}};

    symbols * out_buf = new symbols[in_buf.size() * 10];

    auto cmp_size = pHuffEncode<std::vector<symbols>, 1>( in_buf, out_buf );

    std::cout << "compressed size: " << cmp_size << std::endl;

    return 0;
}