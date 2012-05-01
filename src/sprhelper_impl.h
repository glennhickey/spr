#ifndef _SPRHELPER_IMPL_H
#define _SPRHELPER_IMPL_H

#include "uptree.h"
#include "brackettable.h"

template<class T>
SprHelper<T>::SprHelper(long size) : _size(size)
{
  _flags = (bool*)malloc(size * sizeof(bool));
}

template<class T>
SprHelper<T>::~SprHelper()
{
  free(_flags);
}

/**
 * Flag leaf labels of a subtree.
 * @param tree Input tree.
 * @param pos Start position of the subtree to flag.
 * @param btable Bracket Table associated with the tree.
 * Used to identifiy the end of the subtree.
 */
template<class T>
void SprHelper<T>::flagSubtree(const UPTree<T>& tree, long pos, 
                               const BracketTable<T>& btable)
{
  memset(_flags, false, _size * sizeof(bool));
  for (long i = pos; i < btable[pos].pos[1]; ++i)
  {
    if (tree[i].myType() == LEAF)
    {
      _flags[tree[i].myLabel()] = true;
    }
  }
}

/**
 * @return The start position of the subtree corresponding to all the un
 * UNflagged leaves.
 */
template<class T>
long SprHelper<T>::getFlaggedInverse(const UPTree<T>& tree, 
                                    const BracketTable<T>& btable)
{
  long first = -1, i;
  long last = tree.size() - 1;
  for (i = 0; i < tree.size(); ++i)
  {
    if (tree[i].myType() == LEAF)
    {
      if (_flags[tree[i].myLabel()] == false)
      {
        if (first < 0)
          first = i;
      }
      else if (first >= 0)
      {
        last = i;
        break;
      }
    }
  }
  
  for (i = first - 1; i > 0; --i)
  {
    if (tree[i].myType() == LB)
    {
      if (btable[i].pos[1] >= last)
        break;
      else
        first = i;
    }
    else if (tree[i].myType() == RB)
      break;
  }

  assert(tree[first].myType() == LB);

  return first;
}

/**
 * Search for the next leaf in the string starting at pos.  Is potentially 
 * O(n) so it may be better to search from both ends or use a bracket table
 * or something. 
 */
template<class T>
T SprHelper<T>::nextLeafLabel(const UPTree<T>& tree, long pos)
{
  for (; pos < tree.size(); ++pos)
    if (tree[pos].myType() == LEAF)
      return tree[pos].myLabel();
  assert(false);
  return -1;
}

#endif
