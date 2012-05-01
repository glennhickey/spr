#ifndef _SPRSEARCH_IMPL_H
#define _SPRSEARCH_IMPL_H

/**
 * Create an SPR search.  Note the default constructor is disabled
 * @param man Tree Manager used to create all trees
 * @param cache Tree Cache
 */
template<class T>
SPRSearch<T>::SPRSearch(TreeManager<T>& man, TreeCache<T>& cache, 
                        unsigned char id) :
  _pool1(&_p1), _pool2(&_p2), _man(man), _valid(man.treeSize()), _kern(man), 
  _cache(cache), _btable1(NULL), _btable2(man.treeSize()), 
  _ftable1(NULL), _ftable2(man.treeSize()), 
  _sprTable1(man.treeSize()), _sprTable2(man.treeSize()), 
  _sprHelper(_kern.numLeaves()), _lookup1(NULL), _id(id), _iter(0)
{
  _p1.reserve(1024);
  _p2.reserve(1024);
  _man.createTree(_treeBuf1);
  _man.createTree(_treeBuf2);
}

template<class T>
SPRSearch<T>::~SPRSearch()
{
  // note that treebuf1 and treebuf2 are lost here (but not leaked)
}

/**
 * Add a copy of the start tree to the pool and update the cache. 
 */
template<class T>
void SPRSearch<T>::setStartTree(const UPTree<T>& t1)
{
  _pool1->clear();
  _pool2->clear();
  _count = _cacheHits = 0;

  UPTree<T> t1Copy;
  _man.createTree(t1Copy);
  t1Copy.duplicate(t1);

  _valid.retrifurcate(t1Copy, _treeBuf1, 0);
  _valid.reorder(_treeBuf1, t1Copy);
  _cache.update(t1Copy, _id, _iter);

#ifdef REKERNELIZE
  _pool1->push_back(std::pair<UPTree<T>, T>(t1Copy, 0));
#else  
  _pool1->push_back(t1Copy);
#endif
}

/**
 * Set the end tree.  A copy will be used both to check if the search
 * has been completed and for rekernelization at step.  
 */
template<class T>
void SPRSearch<T>::setEndTree(const UPTree<T>& t2)
{
  UPTree<T> t2Copy;
  _man.createTree(t2Copy);
  t2Copy.duplicate(t2);

  _valid.retrifurcate(t2Copy, _treeBuf1, 0);
  _valid.reorder(_treeBuf1, t2Copy);
  _cache.update(t2Copy, !_id, _iter);

  _kern.setT2(t2Copy);
}

/**
 * Perform one iteration of the search.  All unique neighbours of trees
 * in the current pool are created and added to a pool for the next iteration.
 * Non-unique neighbours are skipped except if their cache flag identifies
 * as an endtree or from a different search, in which case the search 
 * terminates.
 * @return true if the search terminates of false otherwise
 */
template<class T>
bool SPRSearch<T>::iterate()
{
  if (!_pool1->empty())
  {
    _pool2->clear();
    ++_iter;
    size_t n = _pool1->size();
    size_t i;

#ifdef SORTPOOL
    for (i = 0; i < n; ++i)
    {
      _kern.setT1(_pool1->at(i).first);
      _pool1->at(i).second = _kern.kernelize();
    }
    if (!_pool1->empty())
      std::sort(&_pool1->at(0), &_pool1->at(_pool1->size() - 1), PairComp());
#endif

    for (i = 0; i < n; ++i)
    {
#ifdef REKERNELIZE      
      _kern.setT1(_pool1->at(i).first);
      
      // OK, this second rekernelization (which happens if SORTPOOL is defined
      // is totally a design flaw) and is due to the fact that flag
      // information was moved out of the trees and longo the flagtables
      // which are not stored.  Luckily, the extra kernelization isn't
      // terribly costly but this should get fixed at some polong...
      _pool1->at(i).second = _kern.kernelize();
#else
      _kern.setT1(_pool1->at(i));
#endif
      // get table information associated with _pool1->at(i) tree.
      // note that even if kernelize() wasn't called, it is still set
      // by _kern.setT1()
      _btable1 = &_kern.getBTable(0);
      _ftable1 = &_kern.getFTable(0);
      _lookup1 = _kern.getLookup(0);

      // evaluate all 2n-6 NNI neighbours of ith tree in pool1
      // unique results go longo pool2
#ifdef REKERNELIZE
      if (evaluateNNI(_pool1->at(i).first))
#else
      if (evaluateNNI(_pool1->at(i)))
#endif
      {
        return true;
      }
      // evaluate SPR neighbours that aren't NNI's of the same tree
      // again putting uniqe results longo pool 2
#ifdef REKERNELIZE
      else if (evaluateSPR(_pool1->at(i).first))
#else
      if (evaluateSPR(_pool1->at(i)))
#endif
      {
        return true;
      }
    }
    std::swap(_pool1, _pool2);
/*
#ifdef REKERNELIZE
    for (size_t j = 0; j < _pool1->size(); ++j)
    {
      _kern.setT1(_pool1->at(j).first);
      _pool1->at(j).second = _kern.kernelize();
      std::cout << *_ftable1 << "\n" << _kern.getFTable(0) << "\n\n";
    }
#ifdef SORTPOOL
    if (!_pool1->empty())
      std::sort(&_pool1->at(0), &_pool1->at(_pool1->size() - 1), PairComp());
#endif
//    if (_pool1->size() > 173)
//      _pool1->setSize(173);
#endif
*/
  }
  return false;
}

/**
 * Visit all the (2n-6) NNI neighbours of a tree. This is a subset of the
 * SPR neighbours and processed separately for counting purposes.  
 * (see proof of Theorem 2.1 in Allen and Steel).
 */ 
template<class T>
bool SPRSearch<T>::evaluateNNI(UPTree<T>& tree)
{
  long i;
  long lsub[2]; // left and right index of left subtree
  long rsub[2]; // left and right index of right subtree
  long nsub[2]; // left and right index of "nesting subtree"
  _nniCount = 0; // count nieghbours for debugging
 
  // nni will generate 2 trees: swap(left subtree, nesting subtree)
  // and swap(right subtree, nesting subtree)
  
  // we reuse the brackettable and tripos info from the kernelization
  long triPos1 = _btable1->getTriPos(1);
  long triPos2 = _btable1->getTriPos(2);

  // start with all the nni's (there are 2 for each bracket pair
  // not including the first and last brackets). 
  for (i = 1; i < tree.size() - 1; ++i)
  {
#ifdef REKERNELIZE
    
    if (_ftable1->isFrozen(i) == true)
    {
      continue;
    }
#endif
    // if we find a left bracket, evaluate the 2 NNI's that
    // it and its matching right bracket correspond to. 
    if (tree[i].myType() == LB)
    {
        // find start and end polongs of nested subtrees
        lsub[0] = i + 1;
        lsub[1] = _btable1->at(i).rPos - 1;
        rsub[0] = _btable1->at(i).rPos;
        rsub[1] = _btable1->at(i).pos[1] - 1;
      
        // find "nesting" subtree
        if (i != triPos1 && i != triPos2)
        {
          // it's to the right
          if (tree[i-1].myType() == LB)
            nsub[0] = _btable1->at(i).pos[1] + 1;
          // it's to the left
          else
            nsub[0] = _btable1->at(i - 1).pos[0];
        }
        else
        {
          // it's the 3rd trifurcation
          if (i == triPos1)
            nsub[0] = triPos2;
          // it's the 2nd trifurcation
          else
            nsub[0] = triPos1;
        }
        nsub[1] = _btable1->at(nsub[0]).pos[1];
      
        // make the first nni tree
        _man.createTree(_sprNeighbour);
        swapCopy(tree, _sprNeighbour, lsub, nsub);
        if (updateCache(_sprNeighbour))
          return true;
     
        // make the second nni tree
        _man.createTree(_sprNeighbour);
        swapCopy(tree, _sprNeighbour, rsub, nsub);
        if (updateCache(_sprNeighbour))
          return true;

        _nniCount += 2;
      }
  }
#ifndef REKERNELIZE
  assert(_nniCount  == 2 * _kern.numLeaves() - 6); 
#endif
  return false;
}

/**
 * Visit all the SPR neighbours that ARE NOT NNI neighbours: ie there are
 * at least 2 edges between the cut edge and regraft edge.  These edges
 * are located with the SPR table.  
 */
template<class T>
bool SPRSearch<T>::evaluateSPR(UPTree<T>& tree)
{
  long i,j;
  _sprCount = 0;

  _sprTable1.loadTree(tree, *_btable1);

  // DEBUG
  long sprTemp = 0;
  
  for (i = 1; i < tree.size() - 1; ++i)
  {
#ifdef REKERNELIZE
    if (_ftable1->isFrozen(i) == true)
    {
      continue;
    }
#endif
    for (j = 0; j < _sprTable1[i]._num; ++j)
    {
#ifdef REKERNELIZE
      if (_ftable1->isFrozen(_sprTable1[i]._pos[j]) == true)
      {
        continue;
      }
#endif
      _man.createTree(_sprNeighbour);
      sprCopy(tree, _sprNeighbour, i, _sprTable1[i]._pos[j], *_btable1, 
              *_ftable1);
      if (updateCache(_sprNeighbour))
      {
        return true;
      }
      ++_sprCount;
      ++sprTemp;
    }
       
    if (tree[i].myType() == LB)
    {
      // 1)reroot with anything inside subtree, flagging subtree first
      _sprHelper.flagSubtree(tree, i, *_btable1);
      _valid.retrifurcate(tree, _treeBuf2, 
                          SprHelper<T>::nextLeafLabel(tree, i));

      // 2)build a new brackettable
      _btable2.loadTree(_treeBuf2);

      // 3)make sure flags are properly perserved
      _ftable2.loadTree(_treeBuf2, _btable2);
      _ftable2.copy(*_ftable1, _lookup1);
      _ftable2.updateRekern();

      // 4)locate left bracket of "outside" subtree.  
      long outSub = _sprHelper.getFlaggedInverse(_treeBuf2, _btable2);

      // 5)rebuild an sprtable for target subtree of retrifurcated tree
      _sprTable2.loadSubtree(_treeBuf2, outSub, _btable2);

      // 6)perform all possible sprs with this subtree
      for (j = 0; j < _sprTable2[outSub]._num; ++j)
      {
#ifdef REKERNELIZE
        if (_ftable2.isFrozen(_sprTable2[outSub]._pos[j]))
        {
          continue;
        }
#endif
        _man.createTree(_sprNeighbour);

/*            std::cout << "SPR CPY SRC= " << _treeBuf2  << "    "
              << outSub << " --> " << _sprTable2[outSub]._pos[j] 
              << std::endl; */

        sprCopy(_treeBuf2, _sprNeighbour, outSub, _sprTable2[outSub]._pos[j], 
                _btable2, _ftable2);
        if (updateCache(_sprNeighbour))
          return true;
        ++_sprCount;
      }
//      std::cout << std::endl;
    }
  }

#ifndef REKERNELIZE
  assert(_sprCount == 
         ((2 * (_kern.numLeaves() - 3)) * 
         (2 * _kern.numLeaves() - 7)) - 
         (2 * _kern.numLeaves() - 6));
#endif

  return false;
}

/**
 * Copies src longo dest while swapping the two subtrees corresponding
 * to the ranges specified by sub1 and sub2.  In other words, performs
 * a single NNI operation.
 * @param src Source tree
 * @param dest Destination tree
 * @param sub1 Positions of first and last elements of subtree to swap with 
 * sub2
 * @param sub2 Positions of first and last elements of subtree to swap with 
 * sub1
 */
template<class T>
void SPRSearch<T>::swapCopy(const UPTree<T>& src, UPTree<T>& dest, 
                            long sub1[2], long sub2[2])
{
  long i, j, k;

  for (i = 0, j = 0; i < src.size(); ++i)
  {
    if (i == sub1[0])
    {
      for (k = sub2[0]; k <= sub2[1]; ++k)
      {
        dest[j++] = src[k];
      } 
      i = sub1[1];
    }
    else if (i == sub2[0])
    {
      for (k = sub1[0]; k <= sub1[1]; ++k)
      {
        dest[j++] = src[k];
      }
      i = sub2[1];
    }
    else
    {
      dest[j++] = src[i];
    }
  }
}

/**
 * Perform a single spr operation on src tree placing the resulting tree in
 * dest.
 * @param src Source tree
 * @param dest Destination tree
 * @param srcPos Position of subtree to prune (in source)
 * @param destPos Position of subtree to regraft (in source)
 * @param btable Brackettable for source tree.
 */
template<class T>
void SPRSearch<T>::sprCopy(const UPTree<T>& src, UPTree<T>& dest, long srcPos,
                           long destPos, const BracketTable<T>& btable,
                           const FlagTable<T>& ftable)
{
  assert(src[srcPos].myType() != RB);
  assert(src[destPos].myType() != RB);

  // DEBUG 
/*  for (long x = srcPos; x <= btable[srcPos].pos[1]; ++x)
    std::cout << src[x];
  std::cout << " --->  ";
  for (long y = destPos; y <= btable[destPos].pos[1]; ++y)
    std::cout << src[y];
  std::cout << std::flush;
*/
  // remove surrounding brackets
  // copy before destination
  // enclose dest and src in surrounding brackets
  
  long destNext = btable[destPos].pos[1] + 1; // next position after destination
  long nestSrc[2];
  long triPos[3];
  long i, j, k;
  btable.getTriPos(triPos);

  // if we a pruning and regrafting an entire trifurcation, we must
  // knock the brackets off of another trifurcation to keep 3 subtrees
  if (srcPos == triPos[0])
  {
    i = src[triPos[1]].myType() == LB ? 1 : 2;
    nestSrc[0] = triPos[i];
    nestSrc[1] = btable[triPos[i]].pos[1];
  }
  else if (srcPos == triPos[1])
  {
    i = src[triPos[2]].myType() == LB ? 2 : 0;
    nestSrc[0] = triPos[i];
    nestSrc[1] = btable[triPos[i]].pos[1];
  }
  else if (srcPos == triPos[2])
  {
    i = src[triPos[1]].myType() == LB ? 1 : 0;
    nestSrc[0] = triPos[i];
    nestSrc[1] = btable[triPos[i]].pos[1];
  }
  // otherwise, nesting brackets are the surrounding brackets
  else if (src[srcPos - 1].myType() == LB)
  {
    nestSrc[0] = srcPos - 1;
    nestSrc[1] = btable[nestSrc[0]].pos[1];
  }
  else
  {
    nestSrc[1] = btable[srcPos].pos[1] + 1;
    nestSrc[0] = btable[nestSrc[1]].pos[0];
  }

  //DEBUG
  assert(src[nestSrc[0]].myType() == LB);
  assert(src[nestSrc[1]].myType() == RB);
#ifdef REKERNELIZE
//  assert(ftable[nestSrc[0]] == false);
//  assert(ftable[nestSrc[1]] == false);
#endif

  j = 0;

  for (i = 0; i < src.size(); ++i)
  {
    if (i == destPos)
    {
      // copy nesting left bracket
      dest[j++] = src[nestSrc[0]];
      // copy src subtree
      for (k = srcPos; k <= btable[srcPos].pos[1]; ++k)
        dest[j++] = src[k];
      // copy left bracket of dest tree
      dest[j++] = src[i];
    }
    else if (i == destNext)
    {
      // copy nesting right bracket
      dest[j++] = src[nestSrc[1]];
      // copy i
      dest[j++] = src[i];
    }
    else if (i != nestSrc[0] && i != nestSrc[1] && 
             (i < srcPos || i > btable[srcPos].pos[1]))
    {
      // if i hasn't already been copied (ie not part of src or its
      // nesting brackets, then copy it
      dest[j++] = src[i];
    }
  }

  // DEBUG
//  std::cout << "  " <<  src << " --> " << dest << " (check)" << std::endl;
}

/**
 * Search for tree in the cache.  If it does not exist, add it to the cache
 * and to the pool for the next iteration.  
 * @return true if tree corresponds to the end tree.
 */
template<class T>
bool SPRSearch<T>::updateCache(UPTree<T>& tree)
{
// CANNOT USE _TREEBUF2
  ++_count;

//  std::cout << "ADDING TO CACHE " << tree << std::endl;
  _valid.retrifurcate(tree, _treeBuf1, 0);
//  std::cout << "RETRIED =       " << _treeBuf1 << std::endl;
  _valid.reorder(_treeBuf1, tree);
//  std::cout << "REORDERED =     " << tree << std::endl;

  typename TreeCache<T>::Result cres = _cache.update(tree, _id, _iter);
  if (cres._hit == 1)
  {
    if (cres._flag != _id)
      return true;
    ++_cacheHits;
    _man.deleteLast();
    }
  else
  {
#ifdef REKERNELIZE
    _pool2->push_back(std::pair<UPTree<T>, T>(tree, 0));
#else
    _pool2->push_back(tree);
#endif
  }
  return false;
}

#endif
