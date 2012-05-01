#ifndef _TREEDISTANCE_H
#define _TREEDISTANCE_H

#include <vector>
#include <string>

#include "extractsubtree.h"
#include "extractsplitsforest.h"

template<class T> class TreeManager;
template<class T> class TreeCache;
template<class T> class Kernelizor;
template<class T> class Validator;
template<class T> class SPRSearch;


/**
 *  Compute SPR-distance between two unrooted NEWICK trees.  This class 
 *  presently takes as input any pair of unrooted trees with the same leaf
 *  set where the number of leaves is less than 2^13 - 1.  The SPR distance
 *  is computed by first kernelizing the input trees then performing two
 *  exhaustive breadth first searches, one beginning at each tree.  A minimal
 *  SPR-distance is obtained when both searches reach a common topolgy. 
 *
 *  BUG:  I don't think distance() currently works without kernelize() being
 *  called first because numleaveskernelized is required...
 *
 *  @author Glenn Hickey
 */
class TreeDistance
{
 public:
  TreeDistance();
  ~TreeDistance();

  size_t numLeaves(size_t) const;
  
  void setMaxIterations(size_t);
  void setInitManagerSize(size_t);
  void setCacheTableSize(size_t);
  
  bool setTree(const std::string&, long);
  long kernelize();
  long distance(bool);
  long splitsForestDistance();
  long syncLeafSet();

  size_t size() const;
  size_t duplicates() const;

 protected:

  void initKernelizeStructures();
  void initSearchStructures();
  void initLeafSetStructures();

  std::vector<std::string> _nameLookup[2]; /// maps leaf names to numbers
  std::string _treeString[2];              /// copy of input tree strings

  size_t _numLeavesKernelized; /// # leaves in kernelized trees
  size_t _maxIterations;       /// max iterations for search
  size_t _cacheTableSize;      /// size of hash table array
  size_t _initManagerSize;     /// size of inital alloc of treemanager
  size_t _numTreesSearched;
  size_t _numCacheHits;
  
  // Support very large input trees for kernelization
  TreeManager<unsigned short>* _bigManager;  /// manager for 16-bit trees
  TreeManager<unsigned short>* _bigManager2;  /// manager for 16-bit trees
  Kernelizor<unsigned short>* _bigKernelizor; /// kernelizor for 16-bit trees
  Validator<unsigned short>* _bigValidator;  /// validator for 16-bit trees
  ExtractSubtree<unsigned short, unsigned short> 
    _bigExtractor;  /// support for supertree extraction

  ExtractSplitsForest  _splitsForest;   /// support for splits heuristic
  
  // all the real work will be done on byte trees
  // (note to self: fix format so byte trees can support
  // larger trees)
  TreeManager<unsigned char>* _manager;  /// byte tree manager
  TreeCache<unsigned char>* _cache;      /// byte tree cache
  Validator<unsigned char>* _validator;  /// byte tree validator
  SPRSearch<unsigned char>* _searcher[2]; /// byte tree spr searches
};


#endif
