#ifndef _CHERRYTABLE_H
#define _CHERRYTABLE_H

template<class T> class UPTree;
template<class T> class BracketTable;

/**
 * Stores the position of cherry neighbour of each leaf in the tree.  Leaves
 * are cherry neighbours if they are two vertices apart in the tree.  In the
 * string representation, this corresponds to leaves that are adjacent.
 * Leaves that correspond to trifurcations are special cases as they are not
 * necessarily adjacent in the string but can be checked for with the bracket
 * table.
 *
 * @author Glenn Hickey
 */
template<class T>
class CherryTable
{
 public:

  CherryTable();
  CherryTable(long);
  ~CherryTable();
  void resize(long);
  
  long loadTree(const UPTree<T>&, const BracketTable<T>&);
  const long* getTable() const;
  void resetNeighbour(long, long);

  long operator[](long) const;

 protected:

  long _size;    /// size of the table
  long* _ntable; /// cherry position of each leaf offset or -1 if dne.
};


#include "cherrytable_impl.h"

#endif
