#ifndef _BUILDTABLE_IMPL_H
#define _BUILDTABLE_IMPL_H

#include <limits>
#include <cstdlib>
#include "uptree.h"


template<class T>
BracketTable<T>::BracketTable() : _table(NULL), _tabSize(0)
{
}

/**
 * @param size Size of table to create.  Note that this is total number of
 * nodes, not number of leaves.
 */
template<class T>
BracketTable<T>::BracketTable(long size) : _tabSize(size)
{
  _table = (SubtreeRec<T>*)malloc(_tabSize * sizeof(SubtreeRec<T>));
  assert(_tabSize > 0);
}

template<class T>
BracketTable<T>::~BracketTable()
{
  free(_table);
}

/**
 * Resize the table if necessary
 * @param size New size for table.
 */
template<class T>
void BracketTable<T>::resize(long size)
{  
  if (_tabSize == 0)
  {
    _table = (SubtreeRec<T>*)malloc(size * sizeof(SubtreeRec<T>));
  }
  else if (size > _tabSize)
  {
    _table = (SubtreeRec<T>*)realloc(_table, size * sizeof(SubtreeRec<T>));   
  }
  _tabSize = size;
  assert(_tabSize > 0);
}

/**
 * Return size of the table
 */
template<class T>
long BracketTable<T>::getSize() const
{
  return _tabSize;
}

/**
 * Get a reference to the table record corresponding to a particular offset
 * @param pos Position of desired record.
 */ 
template<class T>
inline SubtreeRec<T>& BracketTable<T>::operator[](long pos)
{
  // DEBUG
  if (pos >= _tabSize)
    assert(pos < _tabSize);
  return _table[pos];
}

/**
 * Get a const reference to the table record corresponding 
 * to a particular offset
 * @param pos Position of desired record.
 */ 
template<class T>
inline const SubtreeRec<T>& BracketTable<T>::operator[](long pos) const
{
  assert(pos < _tabSize);
  return _table[pos];
}

/**
 * Get a const reference to the table record corresponding 
 * to a particular offset.  Easier to use on polongers than oeprator[].
 * @param pos Position of desired record.
 */ 
template<class T>
inline const SubtreeRec<T>& BracketTable<T>::at(long pos) const
{
  assert(pos < _tabSize);
  return _table[pos];
}

/**
 * Create a table corresponding to a given tree
 */
template<class T>
long BracketTable<T>::loadTree(const UPTree<T>& tree)
{
  // loadSubtree sets tripos
  long next = loadSubtree(tree, 1, 0);
  next = loadSubtree(tree, _triPos[0] + next, 1);
  loadSubtree(tree, _triPos[1] + next, 2);

  // TODO: return something more meaningful
  return 0;
}

/**
 * Return a const reference to the entire table contents.
 */
template<class T>
inline const SubtreeRec<T>* BracketTable<T>::getTable() const
{
  return _table;
}
 
/**
 * Get the trifurcation number of the subtree containing the token 
 * at a specified position.
 */
template<class T>
inline long BracketTable<T>::getTriPos(long pos) const 
{
  if (pos < _triPos[1])
    return 0;
  if (pos < _triPos[2])
    return 1;
  return 2;
}

/**
 * Get a copy of the start positions of each of the 3 
 * subtrees corresponding to the trifurcation.
 */
template<class T>
inline void BracketTable<T>::getTriPos(long tripos[3]) const
{
  memcpy(tripos, _triPos, sizeof(_triPos));
}

/**
 * Get the start positions of each of the 3 subtrees corresponding to the
 * trifurcation.
 */
template<class T>
inline const long* BracketTable<T>::getTriPos() const
{
  return _triPos;
}

/**
 * Load a subtree longo the table.
 * @param tree Input tree
 * @param pos Start position of subtree
 * @num trifurcation number corresponding to this tree (0,1,2)
 */
template<class T>
long BracketTable<T>::loadSubtree(const UPTree<T>& tree, long pos, long num)
{
  assert(_stack.empty());
  long state;
  long i = pos;
  T idx, mval;
  _triPos[num] = pos;

  do 
  {
    assert(i < tree.size());
    
    // left bracket: 
    if (tree[i].myType() == LB)
    {
      // update position of right subtree
      if (!_stack.empty())
      {
        idx = _stack.top();
        if (i != _table[idx].pos[0] + 1)
          _table[idx].rPos = i;
      }
      // create a new entry in the table.  
      _table[i].min[0] = std::numeric_limits<T>::max();
      _table[i].min[1] = _table[i].min[0];
      _table[i].pos[0] = i;
      _table[i].state = 0;
      _stack.push(i);
    }
    // right bracket
    else if (tree[i].myType() == RB) 
    {
      // find minium of current subtree
      idx = _stack.top();
      _table[idx].pos[1] = i;
      mval = std::min(_table[idx].min[0], _table[idx].min[1]);
      // create new table entry
      _table[i] = _table[idx];
      // close it and updatedcontaining subtree
      _stack.pop();
      if (!_stack.empty())
      {
        idx = _stack.top();
        state = _table[idx].state;
        _table[idx].min[state] = std::min(mval, _table[idx].min[state]);
        _table[idx].state = 1;
      }
    }
    // leaf
    else
    {
      if (i != pos)
      {
        // update min value
        idx = _stack.top();
        state = _table[idx].state;
        _table[idx].min[state] = 
          std::min(tree[i].myLabel(), _table[idx].min[state]);
        _table[idx].state = 1;
        // update position of right subtree
        _table[idx].rPos = i;
        // add leaf entry to table
      }
      _table[i].pos[0] = _table[i].pos[1] = _table[i].rPos = i;
      _table[i].min[0] = _table[i].min[1] = tree[i].myLabel();
    }
    ++i;
  } while (!_stack.empty());


/*
  // DEBUG
  std:: cout << "table validator" <<endl;
  for (i = _table[pos].pos[0]; i <= _table[pos].pos[1]; ++i)
  {
    if (tree[i].myType() != RB)
    {
      cout << "i=" << i << ": lbpos = " << _table[i].pos[0] 
           << " rspos = " << _table[i].rPos
           << " rbpos = " << _table[i].pos[1]
           << " lmin = " << _table[i].min[0]
           << " rmin = " << _table[i].min[1] << endl;
    }
  }
*/
  return i - pos;
}

#endif
