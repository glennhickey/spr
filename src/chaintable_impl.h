#ifndef _CHAINTABLE_IMPL_H
#define _CHAINTABLE_IMPL_H

#include <cmath>
#include "uptree.h"
#include "brackettable.h"

template<class T>
ChainTable<T>::ChainTable() : _size(0), _ntable(NULL)
{
}

/**
 * @param size Size of table to create.
 */
template<class T>
ChainTable<T>::ChainTable(long size) : _size(size)
{
  _ntable = (NList*)malloc(size * sizeof(NList));
}

template<class T>
ChainTable<T>::~ChainTable()
{
  free(_ntable);
}

/** 
 * Resize the table if necessary.
 */
template<class T>
void ChainTable<T>::resize(long size)
{
  if (_size == 0)
  {
    _ntable = (NList*)malloc(size * sizeof(NList));
  }
  else if (size > _size)
  {
    _ntable = (NList*)realloc(_ntable, 
                                         size * sizeof(NList));
  }
  _size = size;
}

/**
 * Return a const reference to the entire table.
 */
template<class T>
inline const typename ChainTable<T>::NList* ChainTable<T>::getTable() const
{
  return _ntable;
}

/**
 * Update the neighbour-list at a particular position
 * @param pos Position of list to update. (should correspond 
 * to a leaf's position)
 * @param neighbours New list of neighbours to apply at pos.
 */
template<class T>
inline void ChainTable<T>::resetNeighbours(long pos, const NList& neighbours)
{
  _ntable[pos] = neighbours;
}

/**
 * Set the number of neighbours to zero.  Useful when we don't 
 * want to visit a node again.
 * @param pos Position to modify. 
 */
template<class T>
inline void ChainTable<T>::removeNeighbours(long pos)
{
  _ntable[pos]._num = 0;
}

/**
 * Remove a position from the neighbourlists of all its chain neighbours
 * @param pos Position to remove.  This position will no longer be in
 * any neighbourlists in the table.
 */
template<class T>
void ChainTable<T>::removeFromNeighbours(long pos)
{
  long i, j, k;
  
  // for each neighbour
  for (i = 0; i < _ntable[pos]._num; ++i)
  {
    k = _ntable[pos]._pos[i];
    // check each of neighbour's links
    for (j = 0; j < _ntable[k]._num; ++j)
    {
      // if link links back to pos
      if (_ntable[k]._pos[j] == pos)
      {
        // remove it by swapping it with last and decrementing count
        _ntable[k]._pos[j] = _ntable[k]._pos[--_ntable[k]._num];
        break;
      }
    }
  }
}

/**
 * Get a const reference to the neighbourlist of a position.
 */
template<class T>
inline 
const typename ChainTable<T>::NList& ChainTable<T>::operator[](long pos) const
{
  return _ntable[pos];
}

/**
 * For each leaf in the tree, find all its chain neighbours and store their
 * positions in the table.  Finding the chain neighbours, with the use
 * of the brackettable is O(1) per leaf.  Loading the whole table is O(n). 
 */
template<class T>
long ChainTable<T>::loadTree(const UPTree<T>& tree, const BracketTable<T>& 
                            btable)
{
  long i, j;
    
  for (i = 0; i < _size; ++i)
    _ntable[i]._num = 0;

  for (i = 0; i < _size; ++i)
  {
    if (tree[i].myType() == LEAF)
    {
      // this loop should only iterate a (very small) COSNTANT number of times,
      // keeping things O(n);
      while (_stack.size())
        _stack.pop();
      for (j = i - 1; j > 0; --j)
      {
        if (tree[j].myType() == RB)
        {
          _stack.push(j);
          if (_stack.size() > 1)
          {
            j = btable[j].pos[0];
            _stack.pop();
          }
        }
        else if (tree[j].myType() == LB)
        {
          if (_stack.size() && tree[_stack.top()].myType() == RB)
            _stack.pop();
          else
            _stack.push(j);
          if (_stack.size() > 1)
            break;
        }
        else
        {
          if (_stack.size() == 1)
          {
            assert(_ntable[i]._num < 4);
            _ntable[i]._pos[_ntable[i]._num++] = j;
          }
        }
      }
      
      // this loop should only iterate a (very small) COSNTANT number of times,
      // keeping things O(n);
      while (_stack.size())
        _stack.pop();
      for (j = i + 1; j < _size; ++j)
      {
        if (tree[j].myType() == LB)
        {
          _stack.push(j);
          if (_stack.size() > 1)
          {
            j = btable[j].pos[1];
            _stack.pop();
          }
        }
        else if (tree[j].myType() == RB)
        {
          if (_stack.size() && tree[_stack.top()].myType() == LB)
            _stack.pop();
          else
            _stack.push(j);
          if (_stack.size() > 1)
            break;
        }
        else
        {
          if (_stack.size() == 1)
          {
            assert(_ntable[i]._num < 4);
            _ntable[i]._pos[_ntable[i]._num++] = j;
          }
        }
      }
    }
  }
  return 0;
}

#endif
