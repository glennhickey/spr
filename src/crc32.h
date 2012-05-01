#ifndef _CRC32_H
#define _CRC32_H

template<class T> class UPTree;

template<class T> 
class Crc32
{
 public:

  static const Crc32* getInstance();
  unsigned int crc(const UPTree<T>&) const;
  
 protected:

  unsigned int _table[256];
  static Crc32* _inst;

 private:
  Crc32();
  Crc32(const Crc32&);
};

#include "crc32_impl.h"

#endif
