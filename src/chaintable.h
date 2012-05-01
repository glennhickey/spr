#ifndef _CHAINTABLE_H
#define _CHAINTABLE_H

#include <stack>

template<class T> class UPTree;
template<class T> class BracketTable;

/**
 * Computes and stores the offsets of pendant leaves that are neighbours in 
 * a chain.  The chain neighbours of a leaf L are all other leaves in the 
 * string that are separated from L by exactly 1 unmatched bracket. This
 * corresponds to a graph theoretic distance of 3 between the vertices. 
 *
 * @author Glenn Hickey
 */
template<class T> 
class ChainTable
{
 public:

  /// The chain neighbours of a leaf.  Note that in a binary tree, there
  /// are at most 4.
  struct NList
  {
    long _num;    /// number of neighbours
    long _pos[4]; /// positions of neighbours
  };
  
  ChainTable();
  ChainTable(long);
  ~ChainTable();
  void resize(long);

  long loadTree(const UPTree<T>&, const BracketTable<T>&);
  const NList* getTable() const;
  void resetNeighbours(long, const NList&);
  void removeNeighbours(long);
  void removeFromNeighbours(long);

  const NList& operator[](long) const;

 protected:

  long _size;                /// Size of the table.
  NList* _ntable;           /// Array of neighbour lists
  std::stack<long> _stack;   /// Stack used for counting brackets
};

#include "chaintable_impl.h"

#endif
