#include <assert.h>

template<class T>
double CVector<T>::_growthFactor = 1.5;

template<class T>
inline CVector<T>::CVector() : _array(NULL), _size(0), _cap(0) {}

template<class T>
inline CVector<T>::CVector(const CVector<T>& v) : _size(v._size), _cap(v._cap)
{
  memcpy(_array, v._array, v._size * sizeof(T));
}

template<class T>
inline CVector<T>::CVector(size_t cap) : _size(0), _cap(cap) 
{
  _array = (T*)malloc(cap * sizeof(T));
}

template<class T>
inline CVector<T>::~CVector() 
{
  free(_array);
}

template<class T>
inline CVector<T>& CVector<T>::operator=(const CVector<T>& v)
{
  _size = v._size;
  _cap = v._cap;
  memcpy(_array, v._array, v._size * sizeof(T));
  return *this;
}

template<class T>
inline T& CVector<T>::operator[](size_t pos)
{
  assert(pos < _cap);
  return _array[pos];
}

template<class T>
inline T& CVector<T>::at(size_t pos)
{
  assert(pos < _size && pos < _cap);
  return _array[pos];
}

template<class T>
inline const T& CVector<T>::operator[](size_t pos) const
{
  assert(pos < _size && pos < _cap);
  return _array[pos];
}

template<class T>
inline const T& CVector<T>::at(size_t pos) const
{
  assert(pos < _size && pos < _cap);
  return _array[pos];
}

template<class T>
inline void CVector<T>::push_back(const T& elem)
{
  if (_size >= _cap)
  {
    _cap = std::max(
      static_cast<size_t>(static_cast<double>(_cap) * _growthFactor),
      _size + 1);
    _array = (T*)realloc(_array, _cap * sizeof(T));
    if (!_array)
      throw 0;
    assert(_size < _cap);
  }
  _array[_size++] = elem;
}

template<class T>
inline void CVector<T>::pop_back()
{
  assert(_size);
  --_size;
}

template<class T>
inline void CVector<T>::reserve(size_t n)
{
  if (n > _cap)
  {
    _cap = static_cast<size_t>(
      static_cast<double>(_cap) * _growthFactor);
    if (n > _cap)
      _cap = n;
    _array = (T*)realloc(_array, _cap * sizeof(T));
    if (!_array)
      throw 0;
    assert(_size < _cap);
  }
}

template<class T>
inline void CVector<T>::clear()
{
  _size = 0;
}

template<class T>
inline size_t CVector<T>::size() const
{
  return _size;
}

template<class T>
inline void CVector<T>::setSize(size_t size)
{
  if (size > _cap)
    reserve(size);
  _size = size;
}

template<class T>
inline bool CVector<T>::empty() const
{
  return _size == 0;
}

