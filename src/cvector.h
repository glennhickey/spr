#ifndef _CVECTOR_H
#define _CVECTOR_H

/**
 * This is a c-style implementation of std::vector.  It is designed for PODs
 * (plain old datatypes) ie objects without constructors and uses realloc()
 * to request new memory.  The assumption is realloc() is potentially 
 * more efficient than new() as it can sometimes increase the reserved memory
 * without having to copy the current contents. 
 *
 * @author Glenn Hickey
 */
template <class T>
class CVector
{
 public:
  CVector();
  CVector(const CVector&);
  CVector(size_t);
  ~CVector();
  
  CVector& operator=(const CVector&);
  T& operator[](size_t);
  T& at(size_t);
  const T& operator[](size_t) const;
  const T& at(size_t) const;
  void push_back(const T&);
  void pop_back();
  void reserve(size_t);
  void clear();
  size_t size() const;
  void setSize(size_t);
  bool empty() const;

  /// Factor to use when resizing: _size = _growthFactor * _size
  static double _growthFactor;
  
 protected:
  
  void grow();

  T* _array;            /// The actual memory block
  size_t _size;   /// The number of elements
  size_t _cap;    /// The present capacity (in number of elements)
};

#include "cvector_impl.h"

#endif
