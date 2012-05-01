#include "crc32.h"
#include "treetok.h"

template<> Crc32<unsigned char>* Crc32<unsigned char>::_inst = NULL;
template<> const unsigned char TreeTok<unsigned char>::RBVal = 126;
template<> const unsigned char TreeTok<unsigned char>::LBVal = 127;
template<> const unsigned short TreeTok<unsigned short>::RBVal = 32766;
template<> const unsigned short TreeTok<unsigned short>::LBVal = 32767;
