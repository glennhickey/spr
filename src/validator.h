#ifndef _VALIDATOR_H
#define _VALIDATOR_H

#include <stack>
#include <iostream>
#include "brackettable.h"

/**
 * Validator class is responsible for verifying that a tree is properly formed.
 * It checks that it is binary, unrooted, and that all the brackets match.
 * Also provides functionality to ensure that the tree is ordered consistently
 * to provide a unique representation for each unique topology.
 *
 * @author Glenn Hickey
 */
template<class T>
class Validator
{
 public:
  Validator(long n);
  ~Validator();
  bool validTree(const UPTree<T>&);
  long subtreeSize(const UPTree<T>&, long);
  long reorder(const UPTree<T>&, UPTree<T>&);
  long retrifurcate(const UPTree<T>&, UPTree<T>&, T);
  const BracketTable<T>& getBTable() const;
  long getSize() const;
    
 protected:

  long reorderSubtree(const UPTree<T>&, long, UPTree<T>&, long);      

  /// Table of matching brackets used to store each bifurcation for quick 
  /// lookup.
  BracketTable<T> _table;
  /// General purpose stack used mostly for counting brackets and tree
  /// traversals.
  std::stack<T> _stack;
  
 private:
  Validator();
};

#include "validator_impl.h"

#endif
