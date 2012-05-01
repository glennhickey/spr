#ifndef _SPRTABLE_H
#define _SPRTABLE_H

#include <stack>
template<class T> class UPTree;
template<class T> class BracketTable;

/**
 * Stores all possible regraft polongs for every subtree in a tree.  
 * There are O(n) potential polongs for each subtree (leaf or left bracket)
 * in the tree.  NNI's are omitted. 
 *
 * Valid SPR's that are not NNI's are subtrees separated by at least two
 * edges.  In the string representation, this corresponds to leaves / 
 * left brackets wtih at least 2 unmatched brackets between them.  
 *
 * @author Glenn Hickey
 */
template<class T> 
class SprTable
{
 public:

  /**
   * List of valid regraft polongs
   */
  struct NList
  {
    long _num;  /// number of neighbours
    long* _pos; /// positions of neighbours
  };
  
  SprTable();
  SprTable(long);
  ~SprTable();
  void resize(long);

  long loadTree(const UPTree<T>&, const BracketTable<T>&);
  long loadSubtree(const UPTree<T>&, long, const BracketTable<T>&);
  const NList* getTable() const;

  const NList& operator[](long) const;

 protected:

  NList* _ntable;        /// list of spr neighbours for each subtree
  long _size;             /// size of trees
  std::stack<long> _stack; /// stack used to count brackets.
};

#include "sprtable_impl.h"

#endif
