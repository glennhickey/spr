#include "extractsplitsforest.h"
#include <set>
#include <algorithm>
#include <limits>
#include <sstream>
#include <string>
#include <iterator>

using namespace std;

Split::Split() : vector<string>(), _left(-1), _right(-1), _flip(false)
{
}

Split::Split(size_t size) : vector<string>(size), 
                                  _left(-1), _right(-1), _flip(false)
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

bool SplitPLess::operator()(const Split* s1, const Split* s2) const
{
  assert(s1 && s2);
  return *s1 < *s2;
}


ostream& operator<<(ostream& os, const Split& split)
{
  size_t i = 0;
  for (; i < split._rp; ++i)
    os << split[i]  << ' ';
  os << " /";
  for (; i < split.size(); ++i)
    os << ' ' << split[i];
  
  return os;
}


ExtractSplitsForest::ExtractSplitsForest()
{
}

ExtractSplitsForest::~ExtractSplitsForest()
{
}

/**
 * Find the common splits forest for a pair of trees
 */

long ExtractSplitsForest::extract(const string& t1, const string& t2, 
                                 vector<string>& f1, 
                                 vector<string>& f2)
{
  // first convert trees to arrays
  vector<string> tree1, tree2;
  for (long i = 0; i < t1.size(); ++i)
  {
    if (t1[i] == ')')
      tree1.push_back(")");
    else if (t1[i] == '(')
      tree1.push_back("(");
    else if (t1[i] != ',')
    {
      long j = t1.find_first_of(")(,", i);
      tree1.push_back(t1.substr(i, j - i));
      i = j - 1;
    }
  }
  for (long i = 0; i < t2.size(); ++i)
  {
    if (t2[i] == ')')
      tree2.push_back(")");
    else if (t2[i] == '(')
      tree2.push_back("(");
    else if (t2[i] != ',')
    {
      long j = t2.find_first_of(")(,", i);
      tree2.push_back(t2.substr(i, j - i));
      i = j - 1;
    }
  }
  _nextLabel = tree1.size();

  // make forest out of non-trivial common splits
  vector<vector<string> > input1, input2;
  vector<vector<string> > output1, output2;
  input1.push_back(tree1);
  input2.push_back(tree2);
  
  for (long i = 0; i < input1.size(); ++i)
  {
    vector<string> sub1a, sub1b, sub2a, sub2b;
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
  assert(output1.size() == output2.size());
  for (long i = 0; i < output1.size(); ++i)
  {
    assert(output1[i].size() == output2[i].size());
    string out1, out2;
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


bool ExtractSplitsForest::breakLCS(const vector<string>& tree1,
                                   const vector<string>& tree2,
                                   vector<string>& subtree1a,
                                   vector<string>& subtree1b,
                                   vector<string>& subtree2a,
                                   vector<string>& subtree2b)
{
  if (tree1.size() < 5)
    return false;

    // DEBUG
/*  cout << endl;
  std::copy(tree1.begin(), tree1.end(), ostream_iterator<string>(cout, " "));
  cout << endl;
  std::copy(tree2.begin(), tree2.end(), ostream_iterator<string>(cout, " "));
  cout << endl << endl << endl;
*/
  set<Split*, SplitPLess> splitSet1, splitSet2;
  vector<Split*> intersection;
  getSplits(tree1, splitSet1);
  getSplits(tree2, splitSet2);
  
  set_intersection(splitSet1.begin(), splitSet1.end(), splitSet2.begin(),
                   splitSet2.end(), back_inserter(intersection), 
                   SplitPLess());

  // find the biggest common split
  long dif = numeric_limits<long>::max();
  vector<Split*>::iterator i, j;
  for (i = intersection.begin(); i != intersection.end(); ++i)
  {
    long v = (long)abs((long)(*i)->_rp - (long)(*i)->size() / 2);
    if (v < dif)
    {
      dif = v;
      j = i;
    }
  }
  
  if (dif != numeric_limits<long>::max())
  {  
    // break the trees 
    splitTree(tree1, **j, subtree1a, subtree1b);
    set<Split*, SplitPLess>::iterator k = splitSet2.find(*j);
    assert(k != splitSet2.end());
    splitTree(tree2, **k, subtree2a, subtree2b);
    assert(subtree1a.size() == subtree2a.size());
    assert(subtree1b.size() == subtree2b.size());
    ++_nextLabel;
  }
  
  // delete the splits
  set<Split*, SplitPLess>::iterator k;
  for (k = splitSet1.begin(); k != splitSet1.end(); ++k)
    delete *k;
  for (k = splitSet2.begin(); k != splitSet2.end(); ++k)
    delete *k;

  // profit
  return !subtree1a.empty() && subtree1a.size() < tree1.size();
}


void ExtractSplitsForest::splitTree(const vector<string>& tree, 
                                    const Split& split,
                                    vector<string>& sub1, 
                                    vector<string>& sub2)
{
  vector<string>* s1 = split._flip ? &sub2 : &sub1;
  vector<string>* s2 = split._flip ? &sub1 : &sub2;

  s1->push_back("(");
  stringstream ss;
  ss << _nextLabel;
  s1->push_back(ss.str());
  for (long i = split._left + 1; i < split._right; ++i)
    s1->push_back(tree[i]);
  s1->push_back(")");

  for (long i = 0; i < split._left; ++i)
    s2->push_back(tree[i]);
  s2->push_back(ss.str());
  for (long i = split._right + 1; i < tree.size(); ++i)
    s2->push_back(tree[i]);
} 



/**
 * Extract the splits from a tree.  There should be one for each INTERNAL
 * edge wich is n-3 or something.
 */
 long ExtractSplitsForest::getSplits(const vector<string>& tree, 
                                   set<Split*, SplitPLess>& splitset)
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
      for (rbpos = i + 1; state; ++rbpos)
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
      split->_right = rbpos - 1;

      // copy in everything outside (i, rbpos)
      for (j = 0; j < i; ++j)
      {
        if (tree[j] != ")" && tree[j] != "(")
        {
          split->push_back(tree[j]);
        }
      }
      for (j = rbpos; j < tree.size(); ++j)
      {
        if (tree[j] != ")" && tree[j] != "(")
        {
          split->push_back(tree[j]);
        }
      }

      // copy everything inside (i, rbpos)
      split->_rp = split->size();
      for (j = i + 1; j < rbpos; ++j)
      {
        if (tree[j] != ")" && tree[j] != "(")
        {
          split->push_back(tree[j]);
        }
      }
      assert(split->size() > split->_rp);

      // sort both halves of the split
      sort(&(*split)[0], &(*split)[split->_rp]);
      sort(&(*split)[split->_rp], 
           &(*split)[split->size()]);
      
      // if minimum leaf is not in the left split then swap the two
      // sides using _temp as a buffer
      if ((*split)[0] > (*split)[split->_rp])
      {
        _temp.clear();
        for (j = 0; j < split->_rp; ++j)
          _temp.push_back((*split)[j]);
        for (j = split->_rp; j < split->size(); ++j)
          (*split)[j - split->_rp] = (*split)[j];
        for (j = 0; j < _temp.size(); ++j)
          (*split)[split->size() - split->_rp  + j] = _temp[j];
        split->_flip = true;
      }
      
      ++k;

      splitset.insert(split);
    }
  }
  return splitset.size();
}

                                       

