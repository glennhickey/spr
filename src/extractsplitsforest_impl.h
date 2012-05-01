#include <set>
#include <algorithm>
#include <limits>

Split::Split() : std::vector<std::string>(), _left(-1), _right(-1)
{
}

Split::Split(size_t size) : std::vector<std::string>(size), 
                                  _left(-1), _right(-1)
{
}

bool Split::operator<(const Split& split) const
{
  if (_rp < split._rp)
    return true;
  if (_rp > split._rp)
    return false;
  for (size_t i = 0; i < _rp; ++i)
  {
    if (this->at(i) < split.at(i))
      return true;
    if (this->at(i) > split.at(i))
      return false;
  }
  return false;
}

bool SplitPLess::operator()(Split* s1, Split* s2)
{
  return *s1 < *s2;
}

template<class T>
std::ostream& operator<<(std::ostream& os, const Split& split)
{
  size_t i = 0;
  for (; i < split._rp; ++i)
    os << (long)split[i]  << ' ';
  os << " /";
  for (; i < split.size(); ++i)
    os << ' ' << (long)split[i];
  
  return os;
}

template<class T>
ExtractSplitsForest<T>::ExtractSplitsForest()
{
}

/**
 * Find the common splits forest for a pair of trees
 */
template<class T>
long ExtractSplitsForest<T>::extract(const UPTree<T>& t1, const UPTree<T>& t2, 
                                    std::vector<std::string>& f1, 
                                    std::vector<std::string>& f2)
{
  // first convert trees to arrays
  std::vector<std::string> tree1, tree2;
  for (long i = 0; i < t1.size(); ++i)
  {
    std::stringstream ss1, ss2;
    ss1 << t1[i];
    tree1.push_back(ss1.str());
    ss2 << t2[i];
    tree2.push_back(ss2.str());
  }

  // make forest out of non-trivial common splits
  std::vector<std::vector<std::string> > input1, input2;
  std::vector<std::vector<std::string> > output1, output2;
  input1.push_back(tree1);
  input2.push_back(tree2);
  
  for (long i = 0; i < input1.size(); ++i)
  {
    std::vector<std::string> sub1a, sub1b, sub2a, sub2b;
    if (breakLCS(input1[i], input2[i], sub1a, sub1b, sub2a, sub2b) == true)
    {
      input1.push_back(sub1a);
      input1.push_back(sub1b);
      input2.push_back(sub2a);
      input2.push_back(sub2b);
    }
    else
    {
      output1.push_back(input1[i]);
      output2.push_back(input2[i]);
    }
  }

  // convert output from vector to string format
  assert(outpu1.size() == output2.size());
  for (long i = 0; i < output1.size(); ++i)
  {
    assert(output1[i].size() == output2[i].size());
    std::string out1, out2;
    for (long j = 0; j < output1[i].size(); ++j)
    {
      if (j > 0 && output1[i][j-1] != "(" && output1[i][j] != ")")
        out1 += ",";
      out1 += output1[i][j];
      
      if (j > 0 && output2[i][j-1] != "(" && output2[i][j] != ")")
        out2 += ",";
      out2 += output2[i][j];
    }
    f1.push_back(out1);
    f2.push_back(out2);
  }
  
  return 0;
}

template<class T>
bool ExtractSplitsForest<T>::breakLCS(const std::vector<std::string>& tree1,
                                     const std::vector<std::string>& tree2,
                                     std::vector<std::string>& subtree1a,
                                     std::vector<std::string>& subtree1b,
                                     std::vector<std::string>& subtree2a,
                                     std::vector<std::string>& subtree2b)
{
  std::set<Split*> splitSet1, splitSet2, longersection;
  getSplits(tree1, splitSet1);
  getSplits(tree2, splitSet2);
  
  std::set_longersection(splitSet1.begin(), splitSet1.end(), splitSet2.begin(),
                        splitSet2.end(), longersection.begin(), SplitPLess());

  // find the biggest common split
  long dif = std::numeric_limits<long>::max();
  std::set<Split*>::iterator i, j;
  for (i = longersection.begin(); i != longersection.end(); ++i)
  {
    long v = (long)abs((long)(*i)->_rp - (long)(*i)->size() / 2);
    if (v < dif)
    {
      dif = v;
      j = i;
    }
  }
  
  if (dif != std::numeric_limits<long>::max())
  {  
    // break the trees 
    splitTree(tree1, *j, subtree1a, subtree1b);
    splitTree(tree2, *splitSet2.find(j), subtree2a, subtree2b);
  }

  // delete the splits
  for (i = splitSet1.begin(); i != splitSet1.end(); ++i)
    delete *i;
  for (i = splitSet2.begin(); i != splitSet2.end(); ++i)
    delete *i;

  // profit
  return !subtree1a.empty() && subtree1a.size() < tree1.size();
}

template<class T>
void ExtractSplitsForest<T>::splitTree(const std::vector<std::string>& tree, 
                                       const Split& split,
                                       std::vector<std::string>& sub1, 
                                       std::vector<std::string>& sub2)
{
  sub1.push_back("(");
  std::stringstream ss;
  ss << _nextLabel;
  sub1.push_back(ss.str());
  for (long i = split._left; i <= split._right; ++i)
    sub1.push_back(tree[i]);
  sub1.push_back(")");

  for (long i = 0; i < split._left; ++i)
    sub2.push_back(tree[i]);
  sub2.push_back(_nextLabel);
  for (long i = split._right + 1; i < tree.size(); ++i)
    sub2.push_back(tree[i]);
} 



/**
 * Extract the splits from a tree.  There should be one for each INTERNAL
 * edge wich is n-3 or something.
 */
template<class T> 
long ExtractSplitsForest<T>::getSplits(const std::vector<std::string>& tree, 
                                      std::set<Split*>& splitset)
{
  size_t i, j;
  size_t k = 0;
  size_t rbpos;
  long state;

  for (i = 1; i < tree.size(); ++i)
  {
    if (tree[i] == "(")
    {
      // first we find the matching right bracket
      state = -1;
      for (rbpos = i; state; ++rbpos)
      {
        if (tree[rbpos] == ")")
          ++state;
        else if (tree[rbpos] == "(")
          --state;
        assert(rbpos < tree.size());
      }
      
      // create the split
      Split* split = new Split((tree.size() + 4 / 3));
      split->_left = i;
      split->_right = rbpos;

      // copy in everything outside (i, rbpos)
      for (j = 0; j < i; ++j)
      {
        if (tree[j] != ")" && tree[j] != "(")
        {
          split->push_back(tree[j]);
        }
      }
      for (j = rbpos + 1; j < tree.size(); ++j)
      {
        if (tree[j] != ")" && tree[j] != "(")
        {
          _splits[k]->push_back(tree[j].myLabel());
        }
      }

      // copy everything inside (i, rbpos)
      _splits[k]->_rp = _splits[k]->size();
      for (j = i + 1; j < rbpos; ++j)
      {
        if (tree[j] != ")" && tree[j] != "(")
        {
          _splits[k]->push_back(tree[j].myLabel());
        }
      }

      // sort both halves of the split
      std::sort(&(*_splits[k])[0], &(*_splits[k])[_splits[k]->_rp]);
      std::sort(&(*_splits[k])[_splits[k]->_rp], 
                &(*_splits[k])[_splits[k]->size()]);
      
      // if minimum leaf is not in the left split then swap the two
      // sides using _temp as a buffer
      if ((*_splits[k])[0] > (*_splits[k])[_splits[k]->_rp])
      {
        _temp.clear();
        for (j = 0; j < _splits[k]->_rp; ++j)
          _temp.push_back((*_splits[k])[j]);
        for (j = _splits[k]->_rp; j < _splits[k]->size(); ++j)
          (*_splits[k])[j - _splits[k]->_rp] = (*_splits[k])[j];
        for (j = 0; j < _temp.size(); ++j)
          (*_splits[k])[_splits[k]->size() - _splits[k]->_rp  + j] = _temp[j];
      }
      
      ++k;

      splitSet->insert(split);
    }
  }
  return splitSet->size();
}

                                       

