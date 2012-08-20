#ifndef _FLAGTABLE_IMPL_H
#define _FLAGTABLE_IMPL_H

#include <cstdlib>
#include "uptree.h"
#include "brackettable.h"

template<class T>
FlagTable<T>::FlagTable() : _size(0), _neis(NULL), _flags(NULL)
{
}

/**
 * @param size Size of table to create.
 */
template<class T>
FlagTable<T>::FlagTable(long size) : _size(size)
{
  _neis = (Neighbours*)malloc(_size * sizeof(Neighbours));
  _flags = (Flag*)malloc(_size * sizeof(bool));
}

template<class T>
FlagTable<T>::~FlagTable()
{
  free(_neis);
  free(_flags);
}

/**
 * Resize table if necessary
 */
template<class T>
void FlagTable<T>::resize(long size)
{
  if (_size == 0)
  {
    _neis = (Neighbours*)malloc(size * sizeof(Neighbours));
    _flags = (Flag*)malloc(size * sizeof(bool));
  }
  else if (size > _size)
  {
    _neis = (Neighbours*)realloc(_neis, size * sizeof(Neighbours));
    _flags = (Flag*)realloc(_flags, size * sizeof(bool));
  }
  _size = size;
}

/**
 * Set all flags in the table
 * @param val Value to assign to all flags.
 */ 
template<class T>
inline void FlagTable<T>::setAll(bool del, bool freeze)
{
  Flag val;
  val._del = static_cast<unsigned char>(del);
  val._freeze = static_cast<unsigned char>(freeze);
  memset(_flags, val, _size);
}

/**
 * @return Position of next unflagged neighbour to the left of pos.
 */
template<class T>
inline long FlagTable<T>::leftPos(long pos) const
{
  return _neis[pos]._left;
}

/**
 * @return Position of next unflagged neighbour to the right of pos.
 */
template<class T>
inline long FlagTable<T>::rightPos(long pos) const
{
  return _neis[pos]._right;
}

/**
 * Find the position of the cherry neighbour of the leaf at position pos
 * in the tree, ignoring all flagged nodes.
 * @return Position of cherry neighbour of -1 if it does not exist.
 */
template<class T>
long FlagTable<T>::cherryPos(long pos) const
{
  if (pos == _triPos[0])
  {
    if (_tree->operator[](_triPos[1]).myType() == LEAF)
      return _triPos[1];
    else if (_tree->operator[](_triPos[2]).myType() == LEAF)
      return _triPos[2];
  }
  else if (pos == _triPos[1])
  {
    if (_tree->operator[](_triPos[0]).myType() == LEAF)
      return _triPos[0];
    else if (_tree->operator[](_triPos[2]).myType() == LEAF)
      return _triPos[2];
  }
  else if (pos == _triPos[2])
  {
    if (_tree->operator[](_triPos[0]).myType() == LEAF)
      return _triPos[0];
    else if (_tree->operator[](_triPos[1]).myType() == LEAF)
      return _triPos[1];
  }
  else if (_tree->operator[](leftPos(pos)).myType() == LEAF)
    return leftPos(pos);
  else if (_tree->operator[](rightPos(pos)).myType() == LEAF)
    return rightPos(pos);

  return -1;
}

/**
 * Assign flag value val to position pos
 */
template<class T>
inline void FlagTable<T>::setFlag(long pos, Flag val)
{
  _flags[pos] = val;
  _flags[pos]._rule = _ruleState;
}

/**
 * Mark flag at pos as frozen
 */
template<class T>
inline void FlagTable<T>::setFrozen(long pos, bool val)
{
  _flags[pos]._freeze = static_cast<unsigned char>(val);
  _flags[pos]._rule = _ruleState;
}

/**
 * Mark flag at pos for deletion
 */
template<class T>
inline void FlagTable<T>::setDel(long pos, bool val)
{
  _flags[pos]._del = static_cast<unsigned char>(val);
  _flags[pos]._rule = _ruleState;
}

/**
 * @return true if position pos is marked as frozen
 */
template<class T>
inline bool FlagTable<T>::isFrozen(long pos) const
{
  return static_cast<bool>(_flags[pos]._freeze);
}


/**
 * @return true if position pos is marked for deletion
 */
template<class T>
inline bool FlagTable<T>::isDel(long pos) const
{
  return static_cast<bool>(_flags[pos]._del);
}

/**
 * @return Boolean flag value at position pos.
 */
template<class T>
inline typename FlagTable<T>::Flag FlagTable<T>::flag(long pos) const
{
  return _flags[pos];
}

/**
 * @return Boolean flag value at position pos.
 */
template<class T>
inline typename FlagTable<T>::Flag FlagTable<T>::operator[](long pos) const
{
  return _flags[pos];
}

/**
 *
 */
template<class T>
void FlagTable<T>::nestPos(long pos, long nest[2]) const
{

}

template<class T>
void FlagTable<T>::setRuleState(unsigned char rule)
{
  assert(rule == 0 || rule == 1);
  _ruleState = rule;
}


/**
 * Load a tree and compute the cherry position of all leaves.
 * @tree Tree to load
 * @btable Brackettable corresponding to tree.
 */
template<class T>
long FlagTable<T>::loadTree(const UPTree<T>& tree, const BracketTable<T>& 
                           btable)
{
  assert(_size == tree.size());
  _tree = &tree;
  _btable = &btable;

  long last = _size - 1;

  _neis[0]._left = -1;
  _neis[0]._right = 1;
  _flags[0]._freeze = 0;
  _flags[0]._del = 0;
  _neis[last]._left = _size - 2;
  _neis[last]._right = -1;
  _flags[last]._freeze = 0;
  _flags[last]._del = 0;

  for (long i = 1; i < last; ++i)
  {
    _neis[i]._left = i - 1;
    _neis[i]._right = i + 1;
    _flags[i]._freeze = 0;
    _flags[i]._del = 0;
  }
  
  memcpy(_triPos, btable.getTriPos(), sizeof(long[3]));
  
  return 0;
}

/**
 * Copy the flags from a tree with a different topology.  This is 
 * acccomplisehd by reflagging each leaf that was flagged for deletion
 * along with its brackets.  Frozen leaves are kept frozen without touching
 * brackets.  updateRekern() may have to be called again.
 */
template<class T>
void FlagTable<T>::copy(const FlagTable<T>& ftable, const long* lookup)
{
  assert(ftable._tree->size() == _tree->size());
  TreeTok<T> tok;
  size_t state = _ruleState;
  
  for (long i = 0; i < _tree->size(); ++i)
  {
    tok = _tree->operator[](i);
    if (tok.myType() == LEAF)
    {
      if (ftable.isDel(lookup[tok.myLabel()]) == true)
      {
        setRuleState(ftable._flags[lookup[tok.myLabel()]]._rule);
        flagLeaf(i);
      }
      _flags[i] = ftable._flags[lookup[tok.myLabel()]];
    }
  }

  _ruleState = state;
}

/**
 * Flag a leaf at position pos.  The structure is then updated to reflect
 * a tree from which this leaf has been deleted.  This means that an edge
 * (pair of brackets) is also flagged and all left/right neighbours are 
 * updated to skip the flagged nodes.  Finally, the start positions of
 * the trifurcation subtrees are updated.  Positions are flagged as 
 * both freeze and delete.  Fine-tuning is done manually using the set
 * methods.
 */
template<class T>
void FlagTable<T>::flagLeaf(long pos)
{
  assert(_tree->operator[](pos).myType() == LEAF);

  flagUpdateNeighbours(pos);

  long i = -1;
  long j = -1;

  // if the leaf we are flagging corresponds to exactly a 
  // trifurcation, it is a special case and we handle accordingly
  // by making sure that _tripos is kept up to date.
  if (pos == _triPos[0])
  {
    i = 0;
    if (_tree->operator[](_triPos[1]).myType() == LB)
      j = 1;
    else if (_tree->operator[](_triPos[2]).myType() == LB)
      j = 2;
  }
  else if (pos == _triPos[1])
  {
    i = 1;
    if (_tree->operator[](_triPos[0]).myType() == LB)
      j = 0;
    else if (_tree->operator[](_triPos[2]).myType() == LB)
      j = 2;
  }
  else if (pos == _triPos[2])
  {
    i = 2;
    if (_tree->operator[](_triPos[0]).myType() == LB)
      j = 0;
    else if (_tree->operator[](_triPos[1]).myType() == LB)
      j = 1;
  }
  if (i >= 0)
  {
    // we want to remove tripos[i] and subdivide tripos[j] longo two
    assert(j >= 0);
    long t = _triPos[j];
    _triPos[i] = rightPos(_triPos[j]);
    flagUpdateNeighbours(t);
    _triPos[j] = getRightPos(_triPos[j]);
    flagUpdateNeighbours(_btable->at(t).pos[1]);

    // ensure tripos are in ascending order
    if (_triPos[1] < _triPos[0])
      std::swap(_triPos[0], _triPos[1]);
    if (_triPos[2] < _triPos[1])
      std::swap(_triPos[1], _triPos[2]);
    if (_triPos[1] < _triPos[0])
    std::swap(_triPos[0], _triPos[1]);
  }

  // leaf is not a trifurcation, update the nesting brackets
  else
  {
    long lpos = leftPos(pos);
    long rpos = rightPos(pos);

    // check left neighbour
    if (_tree->operator[](lpos).myType() == LB)
    {
      // update tripos if necessary
      if (_triPos[0] == lpos)
        _triPos[0] = rightPos(lpos);
      else if (_triPos[1] == lpos)
        _triPos[1] = rightPos(lpos);
      else if (_triPos[2] == lpos)
        _triPos[2] = rightPos(lpos);

      //flag it and its matching right bracket
      flagUpdateNeighbours(lpos);
      flagUpdateNeighbours(_btable->operator[](lpos).pos[1]);
    }
    // check right neighbour
    else if (_tree->operator[](rpos).myType() == RB)
    {
      lpos = _btable->operator[](rpos).pos[0];

      // update tripos if necessary
      if (_triPos[0] == lpos)
        _triPos[0] = rightPos(lpos);
      else if (_triPos[1] == lpos)
        _triPos[1] = rightPos(lpos);
      else if (_triPos[2] == lpos)
        _triPos[2] = rightPos(lpos);

      //flag it and its matching left bracket
      flagUpdateNeighbours(rpos);
      flagUpdateNeighbours(lpos);
    }
    else
    {
      //DEBUG
      //std::cout << " pos = " << pos << " tripos = " << _triPos[0] << " "
      //          << _triPos[1] << " " << _triPos[2] << "\n";
      assert(false);
    }
  }
}

/**
 * Set the flag to freeze & delete at a position and update its 
 * neighbours' neighbours to skip it.
 */
template<class T>
void FlagTable<T>::flagUpdateNeighbours(long pos)
{
  assert(isDel(pos) == false);

  // flag the leaf
  setFrozen(pos, true);
  setDel(pos, true);
  _flags[pos]._rule = _ruleState;

  // update right neighbour of left neighbour
  if (pos > 0)
  {
    _neis[leftPos(pos)]._right = rightPos(pos);
    assert(isDel(_neis[leftPos(pos)]._right) == false);
  }

  // update left neighbour of right neighbour
  if (pos < _size - 1)
  {
    _neis[rightPos(pos)]._left = leftPos(pos);
    assert(isDel(_neis[rightPos(pos)]._left) == false);
  }

}

template<class T>
void FlagTable<T>::updateRekern()
{
  long i;
  TreeTok<T> tok;
  
  // unfreeze outer brackets of subtrees flagged by rule 1.
  // even though these are to be deleted in a kernelization,
  // they can still be pruned and regrafted as a whole after a
  // REkernelization.
  for (i = 0; i < _tree->size(); ++i)
  {
    tok = _tree->operator[](i);
    // if it is a left bracket flagged by rule 1
    if (tok.myType() == LB && isDel(i) && _flags[i]._rule == 0)
    {
      // remove the freeze flag
      assert(isFrozen(i));
      setFrozen(i, false);
      
      // jump out of subtree
      i = _btable->at(i).pos[1] - 1;
    }
  }

  // do the chains
}


/**
 * Finds the right nested subtree of a subtree, making sure to ignore flagged
 * elements.
 * @param pos Position of the leftbracket of the subtree
 * @return position of the right subtree of the input subtree
 */
template<class T>
long FlagTable<T>::getRightPos(long pos)
{
  for (long i = pos + 1; ; ++i)
  {
    assert(i < _btable->at(pos).pos[1]);
    if (!isDel(i) && _tree->operator[](i).myType() != RB)
    {
      assert(i < _btable->at(pos).pos[1]);
      for (long j = _btable->at(i).pos[1] + 1; ; ++j)
      {
        if (!isDel(j) && _tree->operator[](j).myType() != RB)
        {
          return j;
        }
      }
      break;
    }
  }
  // there is no reason to ever reach this polong
  return -1;
}


template<class T>
std::ostream& operator<<(std::ostream& os, const FlagTable<T>& ftable)
{
  TreeTok<T> tok;
  TreeTok<T> prev;
  for (long i = 0; i < ftable._tree->size(); ++i)
  {
    tok = ftable._tree->operator[](i);

    if (i > 0 && prev.myType() != LB && tok.myType() != RB)
      os << ',';
    
    if (ftable.isFrozen(i) && ftable.isDel(i))
      os << '*';
    else if (ftable.isFrozen(i))
      os << 'F';
    else if (ftable.isDel(i))
      os << 'D';
    else
      os << (*ftable._tree)[i];

    if ((ftable.isFrozen(i) || ftable.isDel(i)) && 
        tok.myType() == LEAF && tok.myLabel() > 9)
      os << '.';

    prev = tok;
  }

  return os;
}

#endif
