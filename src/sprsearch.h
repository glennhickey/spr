#ifndef _SPRSEARCH_H
#define _SPRSEARCH_H

#include "cvector.h"

#include "uptree.h"
#include "validator.h"
#include "kernelizor.h"
#include "treecache.h"
#include "sprtable.h"
#include "sprhelper.h"
#include "flagtable.h"

#define MAX_ITERATION 32

/**
 * Breadth first search of SPR-neighbour graph.  Each tree is checked against
 * a cache.  If the tree already exists in the cache, it is ignored unless
 * it has been flagged as an end tree in which case the search terminates.
 * The search works in iterations.  Eeach successive call to iterate will
 * evaluate trees of 1 distance greater from the start tree.
 *
 * @author Glenn Hickey
 */
template <class T>
class SPRSearch
{
 public:

#ifdef REKERNELIZE
  typedef CVector<std::pair<UPTree<T>, T> > TreePool;
  struct PairComp { bool operator()(const std::pair<UPTree<T>, T>& p1,
                                    const std::pair<UPTree<T>, T>& p2) const {
    return p1.second > p2.second;}
  };
#else
  typedef CVector<UPTree<T> > TreePool;
#endif

  SPRSearch(TreeManager<T>&, TreeCache<T>&, unsigned char);
  ~SPRSearch();

  void setStartTree(const UPTree<T>&);
  void setEndTree(const UPTree<T>&);
  bool iterate();

 protected:

  bool evaluateNNI(UPTree<T>&);
  bool evaluateSPR(UPTree<T>&);
  void swapCopy(const UPTree<T>&, UPTree<T>&, long[2], long[2]);
  void sprCopy(const UPTree<T>&, UPTree<T>&, long, long, const BracketTable<T>&,
               const FlagTable<T>&);
  bool updateCache(UPTree<T>&);

  TreePool* _pool1;  /// Unique trees obtained in the previous iteration
  TreePool* _pool2;  /// Unique trees obtained in the present iteration
  TreePool _p1;      /// A pool of trees
  TreePool _p2;      /// A pool of trees
  TreeManager<T>& _man;  /// Tree manager used to create new trees
  Validator<T> _valid;   /// Validator used to reorder and retrifurcate
  Kernelizor<T> _kern;   /// Kernelizor used for rekernelization
  TreeCache<T>& _cache;  /// Tree cache to test for duplicates
  
  UPTree<T> _sprNeighbour; /// A reference to the neighbour currently analazyed
  UPTree<T> _treeBuf1;     /// General purpose temporary tree
  UPTree<T> _treeBuf2;     /// General purpose tempomrary tree
  const BracketTable<T>* _btable1;  /// Bracket Table of current tree
  BracketTable<T> _btable2;         /// Bracket Table of retrifurcated tree
  const FlagTable<T>* _ftable1;  /// Flag Table of current tree
  FlagTable<T> _ftable2;         /// Flag Table of retrifurcated tree
  SprTable<T> _sprTable1;      /// SPR edges of current tree
  SprTable<T> _sprTable2;      /// SPR edges of retrifurcated tree
  SprHelper<T> _sprHelper;     /// Lookup subtrees after retrifurcation
  const long* _lookup1;   /// Position - label map forresponding to _btable1
  
  unsigned char _id;  /// ID of this class.  Trees in the cache that do not 
                      /// share this id are considered End trees. (0 or 1)
  short _iter;        /// Current iteration.

 public:

  size_t _nniCount;      /// NNI neighbours processed (DEBUG ONLY)
  size_t _sprCount;      /// SPR neighbours processed (DEBUG ONLY)
  
  size_t _count;     /// Number of unique trees visited.
  size_t _cacheHits; /// Number of duplicate trees skipped.
 private:
  SPRSearch();
  
};

#include "sprsearch_impl.h"

#endif
