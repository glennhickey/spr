#ifndef _CHERRYTABLE_IMPL_H
#define _CHERRYTABLE_IMPL_H

#include "uptree.h"
#include "brackettable.h"

template<class T>
CherryTable<T>::CherryTable() : _size(0), _ntable(NULL)
{
}

/**
 * @param size Size of table to create.
 */
template<class T>
CherryTable<T>::CherryTable(long size) : _size(size)
{
  _ntable = (long*)malloc(size * sizeof(long));
}

template<class T>
CherryTable<T>::~CherryTable()
{
  free(_ntable);
}

/** 
 * Resize the table if necessary.
 */
template<class T>
void CherryTable<T>::resize(long size)
{
  if (_size == 0)
  {
    _ntable = (long*)malloc(size * sizeof(long));
  }
  else if (size > _size)
  {
    _ntable = (long*)realloc(_ntable, size * sizeof(long));
  }
  _size = size;
}

/**
 * Return a const reference to the entire table.
 */
template<class T>
inline const long* CherryTable<T>::getTable() const
{
  return _ntable;
}

/** 
 * Reset the cherry nieghbour at position pos to val.
 * @param pos Position to reset
 * @param val Offset of new neighbour
 */
template<class T>
inline void CherryTable<T>::resetNeighbour(long pos, long val)
{
  _ntable[pos] = val;
}

/**
 * Get the offset of the cherry neighbour of the leaf at a given position
 * @return offset of cherry neigbhoru or -1 if it doesn't exist
 */
template<class T>
inline long CherryTable<T>::operator[](long pos) const
{
  return _ntable[pos];
}

/**
 * For leaf, find position of its cherry if it exists.  The cherry is 
 * immediately adjacent in the string except if the leaf corresponds to
 * an entire subtree in the trifurcation, then its cherry can another
 * trifurcation.  Note that we don't care about the degenerate case 
 * of a 3-leaf tree where each leaf has two cherry neighbours as such
 * a tree is never kernelized.
 */
template<class T>
long CherryTable<T>::loadTree(const UPTree<T>& tree, const BracketTable<T>& 
                             btable)
{
  long triPos[3];
  btable.getTriPos(triPos);

  for (long i = 0; i < _size; ++i)
  {
    _ntable[i] = -1;
    if (tree[i].myType() == LEAF)
    {
      // Check if left or right neighbour in string is a leaf
      if (i > 0 && tree[i - 1].myType() == LEAF)
      {
        _ntable[i] = i - 1;
      }
      else if (i < _size - 1 && tree[i + 1].myType() == LEAF)
      {
        _ntable[i] = i + 1;
      }

      // Check if leaf is part of a cherry formed by two different
      // trifurcations
      else if (i == triPos[0])
      {
        if (tree[triPos[1]].myType() == LEAF)
        {
          _ntable[i] = triPos[1];
        }
        else if (tree[triPos[2]].myType() == LEAF)
        {
          _ntable[i] = triPos[2];
        }
      }
      else if (i == triPos[1])
      {
        if (tree[triPos[0]].myType() == LEAF)
        {
          _ntable[i] = triPos[0];
        }
        else if (tree[triPos[2]].myType() == LEAF)
        {
          _ntable[i] = triPos[2];
        }
      }
      else if (i == triPos[2])
      {
        if (tree[triPos[0]].myType() == LEAF)
        {
          _ntable[i] = triPos[0];
        }
        else if (tree[triPos[1]].myType() == LEAF)
        {
          _ntable[i] = triPos[1];
        }
      }
    }
  }
  return 0;
}

#endif
