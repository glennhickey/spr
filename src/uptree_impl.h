#ifndef _UPTREE_IMPL_H
#define _UPTREE_IMPL_H

#include <cassert>
#include <cstring>
#include <cstdlib>
#include "treetok.h"
#include "treemanager.h"

/**
 * Return reference to a token in tree.
 * @param pos Position of token to get.
 */
template<class T>
inline TreeTok<T>& UPTree<T>::operator[](long pos) 
{
  assert(_man);
  // DEBUG
  if (pos >= size())
  {
    assert(pos < size());
  }
  return _man->operator[](_os + pos);
}

/**
 * Return copy of a token in tree.
 * @param pos Position of token to get.
 */
template<class T>
inline TreeTok<T> UPTree<T>::operator[](long pos) const
{
  assert(_man);
  // DEBUG
  if (pos >= size())
    assert(pos < size());
  return _man->operator[](_os + pos);
}

/**
 * Duplicate another tree
 * @param tree Tree to duplicate
 */
template<class T>
inline void UPTree<T>::duplicate(const UPTree<T>& tree)
{
  assert(_man == tree._man);
  for (long i = 0; i < tree.size(); ++i)
    this->operator[](i) = tree[i];
}

/**
 * Set flags of all tokens in tree to 0/false
 */
template<class T>
inline void UPTree<T>::stripFlags()
{
  for (long i = 0; i < size(); ++i)
    this->operator[](i).setFlag(false);
}

/**
 * Get the size of the tree.  Note that this is the number of tokens
 * available for this tree and does not correspond to the actual contents.
 * Also note, that this is the total number of tokens and not the number of 
 * leaves.
 */
template<class T>
inline long UPTree<T>::size() const
{
  assert(_man);
  return _man->treeSize();
}

/**
 * Search for a label in the tree and return its position.
 * @param label Leaf label to search for
 * @return position of label or -1 if it is not found.
 */
template<class T>
inline long UPTree<T>::getPos(T label) const
{
  for (long i = 1; i < size(); ++i)
    if (this->operator[](i).myType() == LEAF &&
        this->operator[](i).myLabel() == label)
      return i;
  return -1;
}

/**
 * Prlong tree as Newick string to an output stream. 
 */
template<class T>
std::ostream& operator<<(std::ostream& os , const UPTree<T>& tree)
{
  long p = 0;
  for (long i = 0; i < tree.size(); ++i)
  {
    if (tree[i].myFlag() == false)
    {
      if (i > 0 && tree[p].myType() != LB && tree[i].myType() != RB)
        os << ',';
      os << tree[i];
      p = i;
    }
  }
  return os;
}

/**
 * Read a Newick tree from a stream.  Note that this method does not support
 * branch lengths or longernal node labels so they must be stripped beforehand
 */
template<class T>
std::istream& operator>>(std::istream& is , UPTree<T>& tree)
{
  char* buf = (char*)malloc((tree.size() * 5) * sizeof(char));
  char nbuf[8];
  is >> buf;
  long n = strlen(buf);
  long rsize = 0;
  long p;
  for (long i = 0; i < n; ++i)
  {
    assert(rsize < tree.size());

    if (buf[i] == '(')
      tree[rsize].setLB();
    else if (buf[i] == ')')
      tree[rsize].setRB();
    else if (buf[i] == ',')
      --rsize;
    else
    {
      p = strcspn(buf + i, "(),");
      assert(p > 0);
      strncpy(nbuf, buf + i, p);
      nbuf[p] = '\0';
      tree[rsize].setLeaf(atoi(nbuf));
      i += p - 1;
    }
    ++rsize;
  }
  assert(rsize <= tree.size());
  free(buf);
  return is;
}

#endif
