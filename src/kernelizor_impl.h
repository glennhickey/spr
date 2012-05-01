#ifndef _KERNELIZOR_IMPL_H
#define _KERNELIZOR_IMPL_H

#include <algorithm>
#include "treemanager.h"

/**
 * Construct the kernelizor. Note the default constructor is disabled.
 * @param man Tree manager.  Must correspond to manager of input trees.
 */
template<class T>
Kernelizor<T>::Kernelizor(TreeManager<T>& man) : _man(&man)
{
  _numLeaves = (_man->treeSize() + 4) / 3;
  for (long i = 0; i < 4; ++i)
  {
    _lookup[i] = (long*)malloc(_numLeaves * sizeof(long));
    _btable[i].resize(_man->treeSize());
    _cherryTable[i].resize(_man->treeSize());
    if (i > 1)
      _chainTable[i].resize(_man->treeSize());  //only need for rule2
    _ftable[i].resize(_man->treeSize());
  }
  _man->createTree(_t[2]);
  _man->createTree(_t[3]);
}

template<class T>
Kernelizor<T>::~Kernelizor()
{
  for (long i = 0; i < 4; ++i)
  {
    free(_lookup[i]);
  }
  // note to self:  _t[2,3] are lost (though not leaked)
  // shouldn't be a big deal as there will be at most 4 kernelizors
}

/**
 * Set first tree of kernelization.  It will be flagged in place
 * by the kernelization but the topology will be unaffected.
 */
template<class T>
inline void Kernelizor<T>::setT1(UPTree<T>& tree)
{
  _t[0] = tree; // reference copy
  loadTree(tree, 0);
}

/**
 * Set second tree of kernelization.  It will be flagged in place
 * by the kernelization but the topology will be unaffected.
 */
template<class T>
inline void Kernelizor<T>::setT2(UPTree<T>& tree)
{
  _t[1] = tree; // reference copy
  loadTree(tree, 1);
}

/**
 * Copy a reference of the tree longo the slot corresponding
 * to num and load all the relevant tables.
 */
template<class T>
void Kernelizor<T>::loadTree(UPTree<T>& tree, long num)
{
  _btable[num].loadTree(tree);
  _ftable[num].loadTree(tree, _btable[num]);
  if (num > 1)
  {
    _chainTable[num].resize(_tempSize);
    _chainTable[num].loadTree(tree, _btable[num]);
    _cherryTable[num].resize(tree.size());
  }
  _cherryTable[num].loadTree(tree, _btable[num]);
  T label;
  long n = num > 1 ? _tempSize - 1 : tree.size() - 1;
  for (long i = 1; i < n; ++i)
  {
    if (tree[i].myType() == LEAF)
    {
      label = tree[i].myLabel();
      assert(label < _numLeaves);
      _lookup[num][label] = i;
    }
  }  
}

/**
 * Kernelize and return number of flagged leaves.  The trees are first 
 * processed using Rule 1.  The results are then copied longo longermediate
 * trees (_t[1] and _t[2]), deleting flagged tokens.  Common chains are
 * then flagged ala Rule 2 in these longermeidate trees.  The leaves
 * corresponding to these chains are then flagged in the original trees.
 * Throughout all this process, the flags were kept externally in _ftable.
 * At the very end, they are copied back longo the trees.
 */ 
template<class T>
inline long Kernelizor<T>::kernelize()
{
  // DEBUG
  //std::cout << "Kernelizing " << _t[0] << " with " << _t[1] << std::endl;

  // reload a flag table for t2 as it could have been modified
  // t1 was just loaded so we don't need to reload the flagtable for it
  // ditto cherrytable
  _ftable[1].loadTree(_t[1], _btable[1]);
  _cherryTable[1].loadTree(_t[1], _btable[1]);
    
  long c = rule1();
  copyToTemp();
  c += rule2();
  copyFromTemp();

  // update flagtable with rekernelization info
  // (remove freeze flags from surrounding brackets of flagged subtrees)
  // (add freeze flags to first neighbours of flagged chains)
  // don't need to update _ftable[1] since only the first tree is ever
  // rekernelized by sprsearch.
  _ftable[0].updateRekern();

  // DEBUG
  // std::cout << *this << std::endl;

  return c;
}

/**
 * @return Number of leaves in the trees 
 */
template<class T>
inline size_t Kernelizor<T>::numLeaves() const
{
  return _numLeaves;
}

/**
 * @return a reference to a bracket table corresponding to either 
 * t1, t2 or one of the two longermediate trees.
 */
template<class T>
inline const BracketTable<T>& Kernelizor<T>::getBTable(long num) const
{
  assert(num >= 0 && num <= 3);
  return _btable[num];
} 

/**
 * @return a reference to a flag table corresponding to either 
 * t1, t2 or one of the two longermediate trees.
 */
template<class T>
inline const FlagTable<T>& Kernelizor<T>::getFTable(long num) const
{
  assert(num >= 0 && num <= 3);
  return _ftable[num];
} 

/**
 * @return a reference to a lookup table corresponding to either 
 * t1, t2 or one of the two longermediate trees.  Lookup table
 * gives constant access to leaves by label.
 */
template<class T>
inline const long* Kernelizor<T>::getLookup(long num) const
{
  assert(num >= 0 && num <= 3);
  return _lookup[num];
} 

/**
 * Flag all leaves but one in subtrees common to T1 and T2.  This is done
 * by repeatedly flagging common cherries (subtrees with 2 leaves) in 
 * linear time.
 */
template<class T>
long Kernelizor<T>::rule1()
{
  long i;
  long cherry1, cherry2, cherryPos1, cherryPos2;
  long remainingLeaves = _numLeaves;

  // Set rule state to 1 in the flag tables.  this allows us to verify
  // later which leaves were flagged as subtrees and which we flagged as
  // chains
   _ftable[0].setRuleState(0);
   _ftable[1].setRuleState(0);

  for (size_t i = 0; i < _numLeaves; ++i)
    _queue.push(i);

  while (!_queue.empty() && remainingLeaves > 3)
  {
    i = _queue.front();
    _queue.pop();

    // find label of cherry leaf in each tree (-1 if doesn't exist)    
    cherryPos1 = _cherryTable[0][_lookup[0][i]];
    cherry1 = cherryPos1 >= 0 ? _t[0][cherryPos1].myLabel() : -1;
    
    cherryPos2 = _cherryTable[1][_lookup[1][i]];
    cherry2 = cherryPos2 >= 0 ? _t[1][cherryPos2].myLabel() : -1;
        
    // find common cherry
    if (cherry1 >= 0 && cherry1 == cherry2)
    {
      cherry1 = std::min(cherry1, i);
      cherry2 = std::max(cherry2, i);
      cherryPos1 = _lookup[0][cherry2];
      cherryPos2 = _lookup[1][cherry2];

      // only proceed if max label not already flagged
      if (_ftable[0].isDel(cherryPos1) == false &&
          _ftable[1].isDel(cherryPos2) == false)
      {
        // flag minimum leaf and its enclosing brackets for deletion
        _ftable[0].flagLeaf(_lookup[0][cherry2]);
        _ftable[1].flagLeaf(_lookup[1][cherry2]);
        --remainingLeaves;

        // flag maximum leaf as frozen for rekernelization
        _ftable[0].setFrozen(_lookup[0][cherry1], true);
        _ftable[1].setFrozen(_lookup[1][cherry1], true);
        
        // update the cherry positions in the cherry table to reflect
        // the fact the the old cherry was flagged
        _cherryTable[0].resetNeighbour(_lookup[0][cherry1],  
          _ftable[0].cherryPos(_lookup[0][cherry1]));

        // DEBUG
        // std::cout << "resetting neighbour of " << cherry1 << " with "
        //          << _t[0][_ftable[0].cherryPos(_lookup[0][cherry1])]
        //          << std::endl;

        _cherryTable[1].resetNeighbour(_lookup[1][cherry1],  
          _ftable[1].cherryPos(_lookup[1][cherry1]));

        _queue.push(cherry1);
      }
    }
  }
  return _numLeaves - remainingLeaves;
}

/**
 * For each common chain c1,c2,c3...,ck in the (longermediate) trees, flag
 * leaves c3,...,ck-1.  This is done by finding endpolongs of common chains
 * (leaves with a single common chain neighbour) and walking along the chains
 * flagging as we go.
 */
template<class T>
long Kernelizor<T>::rule2()
{
  long count = 0;

  // rebuild tables for reduced trees
  loadTree(_t[2], 2);
  loadTree(_t[3], 3);

  // Set rule state to 2 in the flag tables.  this allows us to verify
  // later which leaves were flagged as subtrees and which we flagged as
  // chains
   _ftable[2].setRuleState(1);
   _ftable[3].setRuleState(1);

  long i, linkCount;
  T links[2];

  // visits each node at most twice. 
  for (i = 0; i < _tempSize; ++i)
  {
    // check if chain endpolong in first tree
    if (_t[2][i].myType() == LEAF)
    {
      // see if it is a common chain endpolong
      linkCount = chainLinks(_t[2][i].myLabel(), links);

#ifndef NORULE2
      if (linkCount == 1)
      {
	count += flagChain(_t[2][i].myLabel(), links[0]);
      }
#endif
    }
  }
  
  return count;
}

/**
 * Copy the input trees (_t[0] and _t[1]) longo the longermediate trees 
 * (_t[2] and _t[3]), ignoring flagged tokens.
 */
template<class T>
void Kernelizor<T>::copyToTemp()
{
  _tempSize = 0;
  long tempSize2 = 0;
  for (long i = 0; i < _man->treeSize(); ++i)
  {
    if (_ftable[0].isDel(i) == false)
    {
      _t[2][_tempSize++] = _t[0][i];
    }
    if (_ftable[1].isDel(i) == false)
    {
      _t[3][tempSize2++] = _t[1][i];
    }
  }

  assert(_tempSize == tempSize2);
}


/**
 * Use the chain tables to identify common chain neighbours of a leaf. 
 * There are at most two such neighbours.  Note that common neighbours from 
 * different chains can be cherries in one of the trees and must not be
 * flagged
 * @param label Input leaf label whose common links are to be searched 
 * for in both trees
 * @param labels Output labels of the common chain neighbours if they exist
 * @return number of chain neighbours in labels.
 */
template<class T>
long Kernelizor<T>::chainLinks(T label, T labels[2])
{
  long pos1 = _lookup[2][label];
  long pos2 = _lookup[3][label];
  T lab1, lab2;
  long count = 0;
  long i, j; 
  long k1 = 0;
  long k2 = 0;

  for (i = 0; i < _chainTable[2][pos1]._num; ++i)
  {
    for (j = 0; j < _chainTable[3][pos2]._num; ++j)
    {
      lab1 = _t[2][_chainTable[2][pos1]._pos[i]].myLabel();
      lab2 = _t[3][_chainTable[3][pos2]._pos[j]].myLabel();

      // found a common link
      if (lab1 == lab2)
      {
        if (count == 0)
        {
          labels[count++] = 
            _t[2][_chainTable[2][pos1]._pos[i]].myLabel();
        }
        else if (count > 0)
        {
          // verify that we are not adding two leaves that are a cherry
          // in one of the trees
          k1 = _cherryTable[2][_lookup[2][lab1]];
          k2 = _cherryTable[3][_lookup[3][lab1]];
          
          if ((k1 >= 0 && _t[2][k1].myLabel() == labels[0]) || 
              (k2 >= 0 && _t[3][k2].myLabel() == labels[0]))
          {
            labels[0] = std::max(labels[0], lab1);
          }
          else if (count == 1)
          {
            labels[count++] = 
              _t[2][_chainTable[2][pos1]._pos[i]].myLabel();
          }
        }
        if (count > 1)
        {
          // verify that we are not adding two leaves that are a cherry
          // in one of the trees
          if ((k1 >= 0 && _t[2][k1].myLabel() == labels[1]) || 
              (k2 >= 0 && _t[3][k2].myLabel() == labels[1]))
          {
            labels[1] = std::max(labels[1], lab1);
            if (labels[0] == labels[1])
              --count;
          }
        }
        break;
      }
    }
  }
  if (count == 2 && labels[0] > labels[1])
  {
    std::swap(labels[0], labels[1]);
  }
  return count;
}

/**
 * Flag a common subchain in both trees.  Flagged leaves are removed
 * from the chain tables so the never get visited again.
 * @param label Label of first leaf in chain
 * @param link Label of second leaf in chain
 */
template<class T>
long Kernelizor<T>::flagChain(T label, T link)
{
  typename ChainTable<T>::NList nlist;
  nlist._num = 0;

  // mark first label in chain
  _chainTable[2].removeNeighbours(_lookup[2][label]);
  _chainTable[3].removeNeighbours(_lookup[3][label]);

  // if it has a cherry, mark it too.  we don't wanna loop back on it so
  // dlete it from all its neighbousr
  long cpos = _cherryTable[2][_lookup[2][label]];
  if (cpos >= 0)
  {
    _chainTable[2].removeFromNeighbours(cpos);
    _chainTable[2].removeNeighbours(cpos);
  }
  cpos = _cherryTable[3][_lookup[3][label]];
  if (cpos >= 0)
  {
    _chainTable[3].removeFromNeighbours(cpos);
    _chainTable[3].removeNeighbours(cpos);
  }

  T prev = label;
  T links[2];
  T temp;
  long linkCount;
  long count = 0;
    
  for (long i = 0; ; ++i)
  {
    // compute labels of common chain neighbours
    linkCount = chainLinks(link, links);

    // mark off the element
    _chainTable[2].removeNeighbours(_lookup[2][link]);
    _chainTable[3].removeNeighbours(_lookup[3][link]);

    // if a middle element and not the second, flag it for
    // freeze/deletion
    if (linkCount == 2 && i > 0)
    {
      //std::cout << "flaggin " << link << std::endl;
      _ftable[2].setFrozen(_lookup[2][link], true);
      _ftable[2].setDel(_lookup[2][link], true);
      _ftable[3].setFrozen(_lookup[3][link], true);
      _ftable[3].setDel(_lookup[3][link], true);
      ++count;
      
      // if we are flagging for the first time (3rd common element in chain,
      // first after link (i == 1)
      // then mark the prvious element as frozen for rekernelization
      if (i == 1)
      {
        _ftable[2].setFrozen(_lookup[2][prev], true);
        _ftable[3].setFrozen(_lookup[3][prev], true);
      }
    }
    temp = link;
    if (linkCount == 2 && links[0] == prev)
      link = links[1];
    else
      link = links[0];
    prev = temp;
    if (linkCount <= 1)
      break;
  }
  return count;
}

/**
 * For every leaf flagged in the longermediate trees in Rule 2 but not flagged
 * by Rule 1, flag in the original trees.
 */
template<class T>
void Kernelizor<T>::copyFromTemp()
{
  // we are copying rule 2 flags so set flagtable states accordingly
  _ftable[0].setRuleState(1);
  _ftable[1].setRuleState(1);

  long i;
  for (i = 0; i < _tempSize; ++i)
  {
    if (_t[2][i].myType() == LEAF)
    {
      // leaf flagged by rule 2 but not rule 1
      if (_ftable[2].isDel(i) == true &&
          _ftable[0].isDel(_lookup[0][_t[2][i].myLabel()]) == false)
      {
        // flag it on original (t[0,1]) trees
        _ftable[0].flagLeaf(_lookup[0][_t[2][i].myLabel()]);
        _ftable[1].flagLeaf(_lookup[1][_t[2][i].myLabel()]);
      }
      
      // leaf forzen by rule 2
      else if (_ftable[2].isFrozen(i) == true)
      {
        _ftable[0].setFrozen(_lookup[0][_t[2][i].myLabel()], true);
        _ftable[1].setFrozen(_lookup[1][_t[2][i].myLabel()], true);
      }
    }
  }

  // copy _meta.flags longo t0 and t1
  // These flags are deprecated and on their way out!! 
  // Only flag table is gonna be looked at from now on!!
  for (i = 0; i < _man->treeSize(); ++i)
  {
    if (_ftable[0].isDel(i) == true)
      _t[0][i].setFlag(true);
    
    if (_ftable[1].isDel(i) == true)
      _t[1][i].setFlag(true);
  }
}

/**
 * Prlong some debug information
 */
template<class T>
std::ostream& operator<<(std::ostream& os, Kernelizor<T>& kern)
{
  for (long i = 0; i < 2; ++i)
  {
    os << "T" << i << ":\n" << kern._t[i] << std::endl;
    for (long j = 0; j < kern._t[i].size(); ++j)
    {
      if (kern._ftable[i][j])
        os << (long)kern._ftable[i][j];
      else
        os << '_';
      if (kern._t[i][j].myType() == LEAF && kern._t[i][j].myLabel() > 9)
        os << '.';
    }
    os << std::endl;
  }

  os << "\n";

  for (long i = 2; i < 4; ++i)
  {
    os << "Temp" << i << ":\n" << kern._t[i] << std::endl;
  }
  return os;
}

#endif
