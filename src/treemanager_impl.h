#ifndef _TREEMANAGER_IMPL_H
#define _TREEMANAGER_IMPL_H

#define DEF_START_SIZE 1024
#define DEF_TREE_SIZE 32
#define GROW_FACTOR 1.5

#include "treetok.h"
#include "uptree.h"

/**
 * @param numLeaves Number of leaves per tree
 * @param numTrees Number of trees to allocate initially.
 */
template<class T>
TreeManager<T>::TreeManager(size_t numLeaves, size_t numTrees) : 
  _treeSize(numLeaves * 3 - 4), _size(0), _cap(numTrees)
{
  _block = (TreeTok<T>*)malloc((_cap * _treeSize) * sizeof(T));
}

template<class T>
TreeManager<T>::~TreeManager()
{
  free(_block);
}

/**
 * @return size of each tree in manager.
 */
template<class T>
inline size_t TreeManager<T>::treeSize() const 
{
  return _treeSize;
}

/**
 * @return number of leaves in each tree in manager
 */
template<class T>
inline size_t TreeManager<T>::numLeaves() const
{
  return (_treeSize + 4) / 3;
}

/**
 * @return number of trees in manager
 */
template<class T>
inline size_t TreeManager<T>::size() const
{
  return _size;
}

/**
 * Erase all the trees (but don't deallocate)
 */
template<class T>
void TreeManager<T>::reset()
{
  _size = 0;
}

/**
 * @return copy of tree token at offset os
 */
template<class T>
inline TreeTok<T> TreeManager<T>::operator[](size_t os) const 
{
  assert(os < _size * _treeSize);
  assert(os < _cap * _treeSize);
  return _block[os];
}

/**
 * @return reference to tree token at offset os
 */
template<class T>
inline TreeTok<T>& TreeManager<T>::operator[](size_t os)
{
  assert(os < _size * _treeSize);
  assert(os < _cap * _treeSize);
  return _block[os];
}

/**
 * Creates a new tree and reserves the appropriate memory, resizing if
 * the capacity is exceeded
 * @tree Output tree
 */
template<class T>
void TreeManager<T>::createTree(UPTree<T>& tree)
{
  if (_size >= _cap)
  {
    _cap = (size_t)(ceil(_cap * GROW_FACTOR));
    _block = (TreeTok<T>*)realloc(_block, (_cap * _treeSize) * sizeof(T));
    if (_block == NULL)
    {
      std::cerr << "Failed to alloc " << (_cap * _treeSize) * sizeof(T) 
					 << std::endl;
      _cap = 0;
      _size = 0;

      throw 0;
    }
    assert(sizeof(TreeTok<T>) == sizeof(T));
  }

  tree._os = _size * _treeSize;
  tree._man = this;
  _size += 1;
}

/**
 * Request a tree at a given offset and update tree to polong to it.
 */
template<class T>
inline void TreeManager<T>::getTree(size_t index, UPTree<T>& tree)
{
  assert(index <= _size);
  tree._os = index * _treeSize;
  tree._man = this;
}

/**
 * Delete (unrserve space corresponding to) the tree most recently created
 * with createTree()
 */
template<class T>
inline void TreeManager<T>::deleteLast()
{
  assert(_size > 0);
  --_size;
}

/**
 * Compare two trees for equality, disregarding flag bit.  Does this 
 * function really belong here?
 */
template<class T>
bool TreeManager<T>::compare(size_t idx1, size_t idx2)
{
  for (size_t i = 0; i < static_cast<size_t>(_treeSize); ++i)
  {
    if ( _block[idx1 + i]._val != _block[idx2 + i]._val)
    {
      return false;
    }
  }
  return true;
}

#endif
