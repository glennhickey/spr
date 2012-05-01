#ifndef _SPRHELPER_H
#define _SPRHELPER_H

template<class T> class BracketTable;
template<class T> class UPTree;

/**
 * Provides quick access to subtrees after a retrifurcation.  Used when
 * enumerating SPR neighbours.  
 *
 * @author Glenn Hickey
 */
template<class T>
class SprHelper
{
 public:
  SprHelper(long);
  ~SprHelper();

  void flagSubtree(const UPTree<T>&, long, const BracketTable<T>&);
  long getFlaggedInverse(const UPTree<T>&, const BracketTable<T>&);
  static T nextLeafLabel(const UPTree<T>&, long);

 protected:

  bool* _flags;  /// leaves corresponding to a subtree are flagged
  long _size;     /// tree size (not leaves)
  
 private:
  SprHelper();
};

#include "sprhelper_impl.h"
#endif
