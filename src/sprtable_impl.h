#ifndef _SPRTABLE_IMPL_H
#define _SPRTABLE_IMPL_H

#include <cmath>
#include "uptree.h"
#include "brackettable.h"

template<class T>
SprTable<T>::SprTable() : _size(0), _ntable(NULL)
{
}

template<class T>
SprTable<T>::SprTable(long size) : _size(size)
{
  _ntable = (NList*)malloc(size * sizeof(NList));
  for (long i = 0; i < size; ++i)
    _ntable[i]._pos = (long*)malloc(size * sizeof(long));
}

template<class T>
SprTable<T>::~SprTable()
{
  for (long i = 0; i < _size; ++i)
    free(_ntable[i]._pos);
  free(_ntable);
}

/**
 * Resize table if necessary.
 */ 
template<class T>
void SprTable<T>::resize(long size)
{
  if (_size == 0)
  {
    _ntable = (NList*)malloc(size * sizeof(NList));
    for (long i = 0; i < size; ++i)
      _ntable[i]._pos = (long*)malloc(size * sizeof(long));
  }
  else if (size > _size)
  {
    _ntable = (NList*)realloc(_ntable, size * sizeof(NList));
    for (long i = 0; i < size; ++i)
      _ntable[i]._pos = (long*)realloc(_ntable[i]._pos, size * sizeof(long));
  }
  _size = size;
}

/**
 * Get a const polonger to the table
 */
template<class T>
inline const typename SprTable<T>::NList* SprTable<T>::getTable() const
{
  return _ntable;
}

/**
 * Get non-NNI spr candidates for a given subtree position
 */
template<class T>
inline 
const typename SprTable<T>::NList& SprTable<T>::operator[](long pos) const
{
  return _ntable[pos];
}

/**
 * Finds the position of all neighbouring subtrees of distance >= 2
 * from each subtree in tree.  Very similar to the chain table
 * but we also count subtrees as well as leaves by flagging left-bracket
 * positions.  O(n^2)
 */
template<class T>
long SprTable<T>::loadTree(const UPTree<T>& tree, const BracketTable<T>& 
                            btable)
{
  long i;
    
  for (i = 0; i < _size; ++i)
  {
    if (tree[i].myType() == LEAF || tree[i].myType() == LB) 
    {
      loadSubtree(tree, i, btable);
    }
    else
    {
      _ntable[i]._num = 0;
    }
  }
  return 0;
}

/**
 * Find spr candidate neighbours of all subtrees in a subtree
 */
template<class T>
long SprTable<T>::loadSubtree(const UPTree<T>& tree, long pos, 
                             const BracketTable<T>& btable)
{
  assert(tree[pos].myType() == LB || tree[pos].myType() == LEAF);

  long i = pos;
  long j;

  while (_stack.size())
    _stack.pop();

  _ntable[i]._num = 0;

  // search to the left.
  for (j = i - 1; j > 0; --j)
  {
    if (tree[j].myType() == RB)
    {
      _stack.push(j);
    }
    else if (tree[j].myType() == LB)
    {
      if (_stack.size() && tree[_stack.top()].myType() == RB)
      {
        _stack.pop();
        if (_stack.size() >= 2)
        {
          _ntable[i]._pos[_ntable[i]._num++] = j;
        }
      }
      else
      {
        if (_stack.size() >= 2)
        {
          _ntable[i]._pos[_ntable[i]._num++] = j;
        }
        _stack.push(j);
      }
    }
    else
    {
      if (_stack.size() >= 2)
      {
        _ntable[i]._pos[_ntable[i]._num++] = j;
      }
    }
  }
      
  // search to the right
  while (_stack.size())
    _stack.pop();
      
  if (tree[i].myType() == LEAF)
    j = i + 1;
  else
    j = btable[i].pos[1] + 1;
  for (; j < _size; ++j)
  {
    if (tree[j].myType() == LB)
    {
      if (_stack.size() >= 2)
      {
        _ntable[i]._pos[_ntable[i]._num++] = j;
      }
      _stack.push(j);
    }
    else if (tree[j].myType() == RB)
    {
      if (_stack.size() && tree[_stack.top()].myType() == LB)
        _stack.pop();
      else
        _stack.push(j);
    }
    else
    {
      if (_stack.size() >= 2)
      {
        _ntable[i]._pos[_ntable[i]._num++] = j;
      }
    }
  }
  return _ntable[i]._num;
}
#endif
