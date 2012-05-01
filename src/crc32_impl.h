
#include "uptree.h"


template<class T>
inline const Crc32<T>* Crc32<T>::getInstance() 
{
  if (_inst == NULL)
    _inst = new Crc32<T>();
  return _inst;
}

/**
 * Hash a tree into an unsigned int using crc-32
 * Based on code from this web page:
 *
 * http://www.faqs.org/faqs/compression-faq/part1/section-26.html
 */
template<class T>
unsigned int Crc32<T>::crc(const UPTree<T>& tree) const
{
  unsigned int i, j, k;
  unsigned char* p;
  unsigned int  len = tree.size() * sizeof(T);
  TreeTok<T> tok;
  
  // preload shift register, per CRC-32 spec
  unsigned int crc = 0xffffffff;  

  for (i = 0; i < len; ++i)
  {
    k = (len - i) % sizeof(T);
    j = -1 + (len - i) / sizeof(T);
    if (k)
      ++j;
    
    tok = tree[j];
    tok.setFlag(false);
    
    p = reinterpret_cast<unsigned char*>(&tok);
    p += sizeof(T) - k - 1;

    crc = (crc << 8) ^ _table[(crc >> 24) ^ *p];
  }
  
  // transmit complement, per CRC-32 spec
  return ~crc;    
}

/**
 * Build auxiliary table for parallel byte-at-a-time CRC-32.
 *
 * http://www.faqs.org/faqs/compression-faq/part1/section-26.html
 */
template<class T>
Crc32<T>::Crc32()
{
 int i, j;
 unsigned int c;

  for (i = 0; i < 256; ++i) 
  {
    for (c = i << 24, j = 8; j > 0; --j)
      c = c & 0x80000000 ? (c << 1) ^ 0x04c11db7 : (c << 1);
    _table[i] = c;
  }
}
