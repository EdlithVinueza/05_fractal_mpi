#include "palette.h"

uint32_t _bswap32(uint32_t a)
{ // de BGRA a ARGB
    return ((a & 0x000000FF) << 24) |
           ((a & 0x0000FF00) << 8) |
           ((a & 0x00FF0000) >> 8) |
           ((a & 0xFF000000) >> 24);
}

/* 
std::vector<uint32_t> color_ramp = {
    _bswap32(0xFF1010FF),
    _bswap32(0xEF1019FF),
    _bswap32(0xE01123FF),
    _bswap32(0xD1112DFF),
    _bswap32(0xC11237FF),
    _bswap32(0xB21341FF),
    _bswap32(0xA3134BFF),
    _bswap32(0x931455FF),
    _bswap32(0x84145EFF),
    _bswap32(0x751568FF),
    _bswap32(0x651672FF),
    _bswap32(0x56167CFF),
    _bswap32(0x471786FF),
    _bswap32(0x371790FF),
    _bswap32(0x28189AFF),
    _bswap32(0x1919A4FF)
};
*/

std::vector<uint32_t> color_ramp = {
    _bswap32(0x0FFF8BFF),
    _bswap32(0x0FF38CFF),
    _bswap32(0x10E88DFF),
    _bswap32(0x10DC8EFF),
    _bswap32(0x11D190FF),
    _bswap32(0x11C591FF),
    _bswap32(0x12BA92FF),
    _bswap32(0x12AE93FF),
    _bswap32(0x13A395FF),
    _bswap32(0x139796FF),
    _bswap32(0x148C97FF),
    _bswap32(0x148098FF),
    _bswap32(0x15759AFF),
    _bswap32(0x15699BFF),
    _bswap32(0x165E9CFF),
    _bswap32(0x16529DFF),
 
};