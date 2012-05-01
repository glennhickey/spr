#ifdef REKERNELIZE
#undef REKERNELIZE
#endif
#ifdef SORTPOOL
#undef SORTPOOL
#endif

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iterator>
#include <assert.h>
#include <sys/timeb.h>

#include "sprsearch.h"

using namespace std;

vector<string> genYuleHarding(long n, long count)
{
  vector<string> results;
  for (; count > 0; --count)
  {  
    vector<long> t;
    t.push_back(-1);
    t.push_back(0);
    t.push_back(1);
    t.push_back(2);
    t.push_back(-2);
  
    for (long i = 3; i < n; ++i)
    {
      long m = t.size();
      long j;
      // choose random position
      for (j = (rand() % (m-2)) + 1; t[j] == -2; j = (rand() % (m-2)) + 1);

      // insert left bracket
      t.insert(t.begin() + j, -1);
    
      // insert leaf
      t.insert(t.begin() + j + 1, i);
    
      // insert right bracket
      if (t[j + 2] >= 0)
        t.insert(t.begin() + j + 3, -2);
      else
      {
        assert(t[j + 2] == -1);
        long balance = -1;
        long k = j + 3;
        for (; balance != 0; ++k)
        {
          if (t[k] == -1)
            --balance;
          else if (t[k] == -2)
            ++balance;
        }
        // hopefully we landed on a right bracket
        assert(t[k-1] == -2);
        t.insert(t.begin() + k, -2);
      }
    }
  
    string s;
    for (size_t i = 0; i < t.size(); ++i)
    {
      if (i > 0 && t[i - 1] != -1 && t[i] != -2)
        s += ',';
      if (t[i] == -1)
        s += '(';
      else if (t[i] == -2)
        s += ')';
      else
      {
        stringstream ss;
        ss << t[i];
        s += ss.str();
      }
    }
  
    results.push_back(s);
  }
  return results;
}

vector<string> genUniform(long n, long count)
{
  TreeManager<unsigned char> man(n, 1024);
  TreeCache<unsigned char> cache(1000000, &man);
  SPRSearch<unsigned char> search(man, cache, 0);
  UPTree<unsigned char> t1, t2;
  man.createTree(t1);
  man.createTree(t2);
  
  stringstream ss;
  ss << genYuleHarding(n, 1).at(0);
  ss >> t1;
  t2.duplicate(t1);

  search.setEndTree(t2);

  // An longernal copy of t2 is made by setEndtree.  We reset t2 to be
  // a reference of this copy:
  t2._os += man.treeSize(); 
  
  // hack t2 to be a tree that we will never find in the search
  // because it has a different label.  note that this will screw
  // up any kernelization tables (but we are not re/kernelizing so that's
  // ok
  unsigned long i;
  for (i = 0; i < t1.size(); ++i)
    if (t2[i].myType() == LEAF)
    {
      t2[i].setLeaf(n);
      break;
    }

  // All temporary trees will have been allocated by now. 
  unsigned long numTemp = man.size();

  search.setStartTree(t1);
  
  // search for trees of up until max distance (n-3).  this necessary
  // covers the entire search space.  now our tree manager contains
  // UB(n). 
  for (i = 0; i < n - 3; ++i)
    search.iterate();

  vector<string> results;
  for (i = 0; i < count; ++i)
  {
    long rval = numTemp + rand() % (man.size() - numTemp);
    t2._os = rval * man.treeSize();
  
    stringstream ss1;
    ss1 << t2;
    results.push_back(ss1.str());
  }
  return results;
}

int main(long argc, char** argv)
{
  timeb tb;
  ftime(&tb);
  srand(tb.time + tb.millitm);

  if (argc != 4 || (argv[3][0] != 'Y' && argv[3][0] != 'U') ||
    atoi(argv[1]) < 4 || atoi(argv[1]) > 10000 || atoi(argv[2]) < 0)
    cerr << "Usage: " << argv[0] << " <NumLeaves> <NumTrees> <Y|U>\n"
         << "Y: Yule Harding\nU: Uniform" << endl;
  else
  {
    vector<string> results;
    if (argv[3][0] == 'Y')
    {
      results = genYuleHarding(atoi(argv[1]), atoi(argv[2]));
    }
    else
    {
      results = genUniform(atoi(argv[1]), atoi(argv[2]));
    }
    copy(results.begin(), results.end(), ostream_iterator<string>(cout, "\n"));
  }
  return 0;
}
