#ifndef _BRACKETTABLE_H
#define _BRACKETTABLE_H

#include <stack>

template<class T> struct UPTree;

/**
 * Record corresponding to the information of one subtree.
 *
 * @author Glenn Hickey
 */
template<class T>
struct SubtreeRec {
  T pos[2]; /// 0: left 1: right
  T rPos;   /// start of right subtree
  T min[2]; /// 0: left 1: right
  long state;/// 0: left 1: right
};

/**
 * The brackettable stores some information from the tree strings for 
 * quick lookup:  For each bracket, the offeset of its matching bracket
 * is stored.  For each pair of matching brackets, the position of the 
 * corresponding bifurcation as well as the minimum leaf label value 
 * contained in each subtree is stored.  Finally, the offset of the start
 * position of each trifurcation in the string is stored.
 *
 * @author Glenn Hickey
 */
template<class T>
class BracketTable
{
 public:
  BracketTable();
  BracketTable(long);
  ~BracketTable();
  void resize(long);
  long getSize() const;
  
  long loadTree(const UPTree<T>&);
  long loadSubtree(const UPTree<T>&, long, long);
  const SubtreeRec<T>* getTable() const;
  SubtreeRec<T>& operator[](long);
  const SubtreeRec<T>& operator[](long) const;
  const SubtreeRec<T>& at(long) const;

  long getTriPos(long) const;
  void getTriPos(long[3]) const;
  const long* getTriPos() const;

 protected:
  std::stack<T> _stack;  /// General purpose stack.  I should change T to long.
  SubtreeRec<T>* _table; /// The actual table structure which is just an array.
  long _tabSize;          /// Size of the table. 
  long _triPos[3];        /// Offset of each subtree of the trifurcation.
};

#include "brackettable_impl.h"

#endif
