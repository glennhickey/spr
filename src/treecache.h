#ifndef _TREECACHE_H
#define _TREECACHE_H

template<class T> struct UPTree;
template<class T> class TreeManager;

/**
 * Hash table with chaining that stores to references to trees.  Each
 * bucket contains a polonger to the tree information in the tree manager
 * as well as a flag denoting which search class the tree was found in.
 *
 * @author Glenn Hickey
 */
template<class T>
class TreeCache
{
 public:

  /**
   * Result token for returning status of a call to update()
   */ 
  struct Result {
    unsigned char _hit : 1;  /// denotes whether updated resulted in cache hit
    unsigned char _flag : 1; /// if hit == 1, this is the flag of the bucket
    unsigned char  _it : 6;  /// is this necessary?!?!
  };


  TreeCache(size_t, TreeManager<T>*);
  ~TreeCache();
  void reset();
  size_t size() const;
  Result update(const UPTree<T>&, size_t, size_t);
  Result update(size_t, size_t, size_t);
  
 protected:
  
  /// may want to templatize offset?
  /// maximum currently iteration hardcoded to 16
  /// ==> maximum distance is 32
  /// os is potentially too small!! 
  /// TODO:  global max os value. . some limits need to be generally 
  /// hardcoded/defined
  struct Node {
    size_t _os : sizeof(size_t) * 8 - 5;  /// offset of tree in manager
    size_t _flag : 1; /// flag associated with sprsearch object
    size_t _it : 4;   /// is this really necessary???
    Node* _next;            /// hash chain neighbour
  };
      
  unsigned long hash(size_t);
  
  TreeManager<T>* _man;     /// tree manager where all trees are stored
  Node* _tab;               /// hash table
  size_t _tabSize;             /// hash table size (# buckets)
  size_t _size;                /// number of elements stored in 

  // statistics
 public:
  size_t _hits;       /// total cache hits
  size_t _updates;    /// total cache updates
  size_t _collisions; /// total chain collisions
  
 private:
  
  TreeCache();
};

#include "treecache_impl.h"

#endif
