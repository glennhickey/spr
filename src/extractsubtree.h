#ifndef _EXTRACTSUBTREE_H
#define _EXTRACTSUBTREE_H

#include <vector>
#include <string>
#include <set>
#include "brackettable.h"
#include "flagtable.h"

/*
 * This class is designed to extract subtrees induced by a subset of leaves
 * from a supertree. 
 *
 * @author Glenn Hickey
 */
template<class T, class U>
class ExtractSubtree
{
 public:
  
  ExtractSubtree();
  ~ExtractSubtree();
  
  bool extract(const UPTree<U>&, const std::vector<std::string>&,
               const UPTree<T>&, const std::vector<std::string>&, 
               std::string&);

 protected:
  
  BracketTable<U> _btable;
  FlagTable<U> _ftable;
  std::set<std::string> _set;
};

#include "extractsubtree_impl.h"

#endif
