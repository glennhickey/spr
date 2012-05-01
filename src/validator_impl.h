#ifndef _VALIDATOR_IMPL_H
#define _VALIDATOR_IMPL_H

#include <limits>
#include <algorithm>

/**
 * @param size Size of table.  Must be large enough to contain
 * each token of the tree being processed.
 */
template<class T>
Validator<T>::Validator(long size) : _table(size)
{

}

template<class T>
Validator<T>::~Validator()
{

}

/**
 * Return a reference to the bracket table as it is sometimes reused
 * in the kernelization.
 */
template<class T>
inline const BracketTable<T>& Validator<T>::getBTable() const 
{
  return _table;
}

/**
 * Get the size of the bracket table.  Corresponds to the maximum size of 
 * a tree that can be validated.
 */ 
template<class T>
inline long Validator<T>::getSize() const 
{
  return _table.getSize();
}

/**
 * Ensure that the tree is composed of a 3 valid binary subtrees. Useful
 * mostly for testing input.
 */
template<class T>
bool Validator<T>::validTree(const UPTree<T>& tree)
{
  if (tree[0].myType() != LB)
    return false;

  long pos = 1;
  long delta, i;
  for (i = 0; i < 3; ++i)
  {
    delta = subtreeSize(tree, pos);
    if (!delta)
      return false;
    pos += delta;
  }
  
  return pos == tree.size() - 1 && tree[pos].myType() == RB;
}

/** returns size of subtree in number of tokens if valid. 0 otherwise
 * Checks if it starts with a (.  
 * Checks that all brackets match.
 * Checks that it is strictly bifurcating.
 * @param tree Input tree
 * @param pos Starting position of subtree to analyze
 */
template<class T>
long Validator<T>::subtreeSize(const UPTree<T>& tree, long pos)
{
  long i = pos;
  
  if (tree[i].myType() == LEAF)
    return 1;

  assert(_stack.empty());

  do 
  {
    if (i >= tree.size())
      return 0;

    // left bracket: push to stack & increase subtree count
    if (tree[i].myType() == LB)
    {
      if (!_stack.empty())
        ++_stack.top();
      _stack.push(0);
    }
    // right bracket: pop from stack
    else if (tree[i].myType() == RB) 
    {
      if (_stack.empty() || _stack.top() != 2)
        return 0;
      _stack.pop();
    }
    // leaf: increase subtree count
    else
    {
      if (_stack.empty() || _stack.top() > 2)
        return 0;
      ++_stack.top();
    }
    ++i;
  } while (!_stack.empty());
  
  if (!_stack.empty())
    while(!_stack.empty()) _stack.pop();

  return i - pos;
}

/**
 * Reorder a subtree to ensure that at each longernal node, the minimum leaf
 * label in the left subtree is smaller than the minimum leaf label in the 
 * right subtree.  Ordering all subtrees in this fashion is essential for
 * a unique representation required by the cache.  This is done in a single
 * scan.
 * @param itree Input tree.
 * @param pos Starting position of subtree in input tree to reorder.
 * @param otree Output tree.
 * @param opos Starting position of subtree in output tree.
 * @return Number of subtrees that were out of order.
 */
template<class T>
long Validator<T>::reorderSubtree(const UPTree<T>& itree, long pos, 
                                    UPTree<T>& otree, long opos)
{
  assert(_stack.empty());

  long idx;
  long swaps = 0;

  _stack.push(pos);

  while (!_stack.empty())
  {
    idx = _stack.top();
    _stack.pop();
    
    // left bracket:  copy left bracket
    // copy min subtree. then copy max subtree
    // using stack so first is last. .. 
    if (itree[idx].myType() == LB)
    {
      otree[opos++] = itree[idx];
      _stack.push(_table[idx].pos[1]);
      if (_table[idx].min[0] < _table[idx].min[1])
      {
        _stack.push(_table[idx].rPos);
        _stack.push(idx + 1);
      }
      else
      {
        ++swaps;
        _stack.push(idx + 1);
        _stack.push(_table[idx].rPos);
      }
    }
    // right bracket: write it and pop the matching left
    else if (itree[idx].myType() == RB)
    {
      otree[opos++] = itree[idx];
    }
    // leaf: copy label longo new tree
    else 
    {
      otree[opos++] = itree[idx];
    }
  }  
  return swaps;
}

/**
 * Ensure that for each longernal node of the tree, the minimum leaf label
 * in its left subtree is smaller than the minimum leaf label in its right
 * subtree.  
 * @param itree Tree to reorder
 * @param otree Tree to which the reordered result is written
 * @return Number of subtrees that had to be reordered.
 */
template<class T>
long Validator<T>::reorder(const UPTree<T>& itree, UPTree<T>& otree)
{
  long triPos[3];
  _table.loadTree(itree);
  _table.getTriPos(triPos);
     
  otree[0] = itree[0];
  
  // collect minimum value of each subtree
  T minVal[3];
  if (itree[triPos[0]].myType() == LEAF)
    minVal[0] = itree[triPos[0]].myLabel();
  else
    minVal[0] = std::min(_table[triPos[0]].min[0], _table[triPos[0]].min[1]);
  if (itree[triPos[1]].myType() == LEAF)
    minVal[1] = itree[triPos[1]].myLabel();
  else
    minVal[1] = std::min(_table[triPos[1]].min[0], _table[triPos[1]].min[1]);
  if (itree[triPos[2]].myType() == LEAF)
    minVal[2] = itree[triPos[2]].myLabel();
  else
    minVal[2] = std::min(_table[triPos[2]].min[0], _table[triPos[2]].min[1]);

  // sort subtrees by their minimum values
  long order[3] = {0, 1, 2};
  if (minVal[order[1]] < minVal[order[0]])
    std::swap(order[0], order[1]);
  if (minVal[order[2]] < minVal[order[1]])
    std::swap(order[1], order[2]);
  if (minVal[order[1]] < minVal[order[0]])
    std::swap(order[0], order[1]);

  // reorder subtrees in sorted order
  long swaps = reorderSubtree(itree, triPos[order[0]], otree, triPos[0]);
  long size = _table[triPos[order[0]]].pos[1] - triPos[order[0]] + 1;
  swaps += reorderSubtree(itree, triPos[order[1]], otree, triPos[0] + size);
  size += _table[triPos[order[1]]].pos[1] - triPos[order[1]] + 1;
  swaps += reorderSubtree(itree, triPos[order[2]], otree, triPos[0] + size);
  
  otree[itree.size() - 1] = itree[itree.size() - 1];

  return swaps;
}

/**
 * Retrifurcation of an unrooted tree is analagous to rerooting a rooted 
 * tree. In order to ensure a unique representation, we enforce that the 
 * leaf with label=0 correspond to the first subtree of the trifurcation.
 * The two subtrees rooted at leaf 0's neighbours will correspond to the
 * second and third trifurcations.
 *
 * Example: 
 * Input:  ((4,5),((0,3),2),(1,6))
 * Output: (0,3,(((4,5),2),(6,1)))
 *
 * This is a rather involved routine that will hopefuly be fully described
 * somewhere in the thesis.  
 *
 * Note that it must be used in conjunction (ie before) reorder to generate
 * a unique representation
 * @param itree Input tree
 * @param otree Tree in which retrifurcated itree will be copied
 * @param root Label of 'root' that will be set as first trifurcation.  I use
 * 0
 * @return 0 if input is already retrifurcated or 1 if work had to be done
 */

template<class T>
long Validator<T>::retrifurcate(const UPTree<T>& itree, UPTree<T>& otree, 
                               T root)
{
  long i, j, k;
  long triPos[3];
  T rPos = 0;

  _table.loadTree(itree);
  _table.getTriPos(triPos);

  // find 'root' leaf
  rPos = itree.getPos(root);
  assert(rPos > 0);

  long minTri = 0;
  if (rPos >= triPos[1])
    minTri = 1;
  if (rPos >= triPos[2])
    minTri = 2;

  // already trifurcated.  do nothing (note it may not be in right order)
  if (rPos == triPos[minTri])
  {
    otree.duplicate(itree);
    return 0;
  }

  j = 0;

  // create new LEFT subtree
  otree[j++] = itree[0];
  otree[j++] = itree[rPos];
  
  // create MIDDLE subtree out of mins neighbour which
  // can be a right leaf, left leaf, right neighbour or left neighbour
  long midLeft = 0;
  long midRight = 0;
  if (itree[rPos + 1].myType() == LEAF ||
      itree[rPos + 1].myType() == LB)
  {
    midLeft = rPos + 1;
    midRight = _table[midLeft].pos[1];
  }
  else if (itree[rPos - 1].myType() == LEAF ||
           itree[rPos - 1].myType() == RB)
  {
    midRight = rPos - 1;
    midLeft = _table[midRight].pos[0];
  }
  for (i = midLeft; i <= midRight; ++i)
  {
    otree[j++] = itree[i];
  } 
  
  // create RIGHT subtree
  
  // flip everything above of min in the nesting
  i = midLeft - 1;
  if (i == rPos)
    --i;
  for(; i >= triPos[minTri]; --i)
  {
    // if entire subtree is within this range, copy as is (don't flip)
    if (itree[i].myType() == RB)
    {
      for (k = _table[i].pos[0]; k <= i; ++k)
        otree[j++] = itree[k];
      i = _table[i].pos[0];
      }
      else 
      otree[j++] = itree[i];
  }
  
  // add all remaining leaves beginning with the original trifurcation
  // subtrees that do not contain min;
  long s1 = rPos < triPos[1] ? 1 : 0;
  long s2 = rPos > triPos[2] ? 1 : 2;
  for (i = triPos[s1]; i <= _table[triPos[s1]].pos[1]; ++i)
  {
    otree[j++] = itree[i];
  }
  for (i = triPos[s2]; i <= _table[triPos[s2]].pos[1]; ++i)
  {
    otree[j++] = itree[i];
  }
  
  // add remaining items from split subtree
  i = midRight + 1;
  if (i == rPos)
    ++i;
  /*
  for (; i <= _table[triPos[minTri]].pos[1]; ++i)
  {
    otree[j++] = itree[i];
  }
  */
  // try backwards:
  long p = i;
  for (i = _table[triPos[minTri]].pos[1]; i >= p; --i)
  {
    // if entire subtree is within this range, copy as is (don't flip)
    if (itree[i].myType() == RB && _table[i].pos[0] > p)
    {
      for (k = _table[i].pos[0]; k <= i; ++k)
        otree[j++] = itree[k];
      i = _table[i].pos[0];
    }
    else 
      otree[j++] = itree[i];
  } 

  // add last bracket
  otree[j++] = itree[itree.size() - 1];
  
  assert(j == itree.size());
    
  return 1;
}


#endif
