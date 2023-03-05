#include <pHuffWriter.h>





// @return 0: write success;
//        -1: write end;
//       <-1: write error;
inline int write( const HuffSeg inHuffData, WriteBufferState& currBufferState )
{
    std::uint64_t curr_huff_seg = *static_cast<std::uint64_t*>(currBufferState.getCurWritePtr());
    const int cur_filled = currBufferState.getCurFilledLen();
    std::uint64_t masked_curr_huff_seg = [curr_huff_seg, cur_filled]()
    {
        switch(cur_filled)
        {
            case 0:
                return 0;
            case 1:
                return curr_huff_seg & 0x8000000000000000;
            case 2:
                return curr_huff_seg & 0xc000000000000000;
            case 3:
                return curr_huff_seg & 0xe000000000000000;
            case 4:
                return curr_huff_seg & 0xf000000000000000;
            case 5:
                return curr_huff_seg & 0xf800000000000000;
            case 6:
                return curr_huff_seg & 0xfc00000000000000;
            case 7:
                return curr_huff_seg & 0xfe00000000000000;
        }
    }();
}


