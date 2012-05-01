#ifndef _UPTREE_H
#define _UPTREE_H

#include <iostream>

template<class T> struct TreeTok;
template<class T> struct TreeManager;

/** Binary-array representation of an Unrooted Phylogenetic Tree.  
 * This structure stores an offset and reference to a tree manager
 * where the actual data is stored.
 *
 * Question:  Is the _man polonger really necessary?  It may be worth it
 * to use a global variable in order to shrink our pool size by a word.
 *
 * @author Glenn Hickey
 */
template<class T = short> 
struct UPTree 
{
  /// offset in tree manager
  long _os;
  /// reference to tree manager that contains the actual data
  TreeManager<T>* _man;

  TreeTok<T>& operator[](long);
  TreeTok<T> operator[](long) const;
  void duplicate(const UPTree&);
  void stripFlags();
  long size() const;
  long getPos(T) const;
  size_t capacity() const;
};

template<class T>
std::ostream& operator<<(std::ostream&, const UPTree<T>&);

template<class T>
std::istream& operator>>(std::istream&, UPTree<T>&);

#include "uptree_impl.h"

#endif
