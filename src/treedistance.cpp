#define DUAL_SEARCH

#include <iterator>
#include <sstream>
#include "treedistance.h"
#include "treemanager.h"
#include "treecache.h"
#include "kernelizor.h"
#include "sprsearch.h"
#include "crc32.h"

static const size_t DEFAULT_MAX_ITERATIONS = 10;
static const size_t DEFAULT_CACHETABLE_SIZE = 10000001;
static const size_t DEFAULT_INITMANAGER_SIZE = DEFAULT_CACHETABLE_SIZE;


using namespace std;

TreeDistance::TreeDistance() :
  _numLeavesKernelized(0),
  _maxIterations(DEFAULT_MAX_ITERATIONS),
  _cacheTableSize(DEFAULT_CACHETABLE_SIZE),
  _initManagerSize(DEFAULT_INITMANAGER_SIZE),
  _bigManager(NULL),
  _bigManager2(NULL),
  _bigKernelizor(NULL),
  _bigValidator(NULL),
  _manager(NULL),
  _cache(NULL),
  _validator(NULL)
{
  _searcher[0] = NULL;
  _searcher[1] = NULL;
}

TreeDistance::~TreeDistance()
{
  delete _bigManager;
  delete _bigManager2;
  delete _bigKernelizor;
  delete _bigValidator;
  delete _manager;
  delete _cache;
  delete _validator;
  delete _searcher[0];
  delete _searcher[1];
}

size_t TreeDistance::numLeaves(size_t i) const
{
  assert(i == 1 || i == 0);
  return _nameLookup[i].size();
}

/**
 * Specify maximum number of iterations
 */
void TreeDistance::setMaxIterations(size_t mi)
{
  _maxIterations = mi;
}

/**
 * Reset the initial manager size.  as a consequence, the other structures
 * must also be hosed.  i can't really think of a reason why this function
 * woudl be useful.
 */
void TreeDistance::setInitManagerSize(size_t size)
{
  if (_initManagerSize != size && _manager != NULL)
  {
    delete _manager;
    delete _cache;
    delete _searcher[0];
    delete _searcher[1];
    _manager = NULL;
    _cache = NULL;
    _searcher[0] = NULL;
    _searcher[1] = NULL;
  }
}

/**
 * Reset the number of buckets in the hash table used by the tree cache
 * as a consequnce, the cache and searchers are out of date
 */
void TreeDistance::setCacheTableSize(size_t size)
{
  if (_cacheTableSize != size && _cache != NULL)
  {
    delete _cache;
    delete _searcher[0];
    delete _searcher[1];
    _cache = NULL;
    _searcher[0] = NULL;
    _searcher[1] = NULL;
  }
}

/**
 * Load a Newick tree
 * @param num Specify T1 or T2 (must be 0 or 1)
 * @return true if successfully loaded, false otherwise
 */
bool TreeDistance::setTree(const string& inString, long num )
{
  assert(num == 0 || num == 1);
  _treeString[num].clear();
  _nameLookup[num].clear();
  
  size_t n = inString.length();
  size_t i;
  size_t k;
  size_t tokCount = 0;
  vector<string>::iterator it;
    
  // first pass : collect names
  for (i = 0; i < n; ++i)
  {
    switch (inString[i])
    {
    case ' ' :
    case '\n':
    case '\t':
    case ';' :
    case '(' :
    case ')' :
    case ',' :
    case ':' :
      break;
    default:
      if (i == 0)
        return false;
      // skip longernal node labels and branch lengths
      if (inString[i - 1] == ')' || inString[i - 1] == ':')
      {
        k = inString.find_first_of(",()", i);
        if (k == string::npos)
          return false;
        i = k - 1;
      }
      else
      {  
        k = inString.find_first_of(",():", i);
        if (k == string::npos)
          return false;
        _nameLookup[num].push_back(inString.substr(i, k - i));
        i = k - 1;
      }
      break;
    }
  }

  // sort names
  sort(_nameLookup[num].begin(), _nameLookup[num].end());

  // check for duplicates
  for (i = 1; i < _nameLookup[num].size(); ++i)
    if (_nameLookup[num][i] == _nameLookup[num][i - 1])
      return false;

  // second pass: copy longo _treeString, stripping branch lengths and
  // replacing names with unique numbers between 0 and numleaves - 1
  for (i = 0; i < n; ++i)
  {
    switch (inString[i])
    {
    case ' ' :
    case '\n':
    case '\t':
    case ';' :
      break;
    case '(' :
    case ')' :
      ++tokCount;
    case ',' :
      _treeString[num] += inString[i];
      break;
    case ':' :
      i = inString.find_first_of(",()", i);
      if (i == string::npos)
        return false;
      i -= 1;
      break;
    default :
      // skip longernal node labesl and branch lengths
      if (inString[i - 1] == ')' || inString[i - 1] == ':')
      {
        k = inString.find_first_of(",()", i);
        if (k == string::npos)
          return false;
        i = k - 1;
      }
      else
      {
        k = inString.find_first_of(",():", i);
        if (k == string::npos)
          return false;
        it = lower_bound(_nameLookup[num].begin(), _nameLookup[num].end(),
                         inString.substr(i, k - i));
        assert(it != _nameLookup[num].end());
        stringstream ss;
        ss << it - _nameLookup[num].begin();
        _treeString[num] += ss.str();
        i = k - 1;
        ++tokCount;
      }
      break;
    }
  }

  // if tree is rooted then unroot it (assume proper format which 
  // will be tested for later on by validator)
  // unrooted:  |t| = 3n -4 (n = numleaves, |t| = numleaves+numbrackets)
  // rooted: |t| = 3n - 2
  if (tokCount == _nameLookup[num].size() * 3 - 2)
  {
    long state = 0;
    string::iterator i = _treeString[num].begin();
    string::iterator n = _treeString[num].end();
    for (++i, --n; i != n; ++i)
    {
      // delete the first left bracket found after the outside bracket
      if (*i == '(')
      {
        if (state == 0)
        {
          i = _treeString[num].erase(i);
          --i;
        }
        --state;
      }
      // delete the rigth bracket that matches the left bracket we deleted
      else if (*i == ')')
      {
        ++state;
        if (state == 0)
        {
          _treeString[num].erase(i);
          break;
        }
      }
    }
  }
  return true;
}

/**
 * Kernelize the two trees loaded with setTree().  
 * @return Number of leaves removed by kernelization.
 */
long TreeDistance::kernelize()
{
  // make sure that some trees ahve been proplerly loaded
  if (_nameLookup[0] != _nameLookup[1] || _nameLookup[0].empty()
      ||_nameLookup[1].empty())
  {
    cerr << "Kernelize error: Input trees have different/empty leaf sets" 
         << endl;
    return -1;
  }

  // make sure that kernelizor is properly allocated
  initKernelizeStructures();
  
  // Create the Trees
  UPTree<unsigned short> t1, t2;
  _bigManager->createTree(t1);
  _bigManager->createTree(t2);

  // Convert strings and validate
  stringstream ss1;
  ss1 << _treeString[0];
  ss1 >> t1;
  stringstream ss2;
  ss2 << _treeString[1];
  ss2 >> t2;

  // DEBUG
  // cout << "T1clean = " << _treeString[0] << "\nT2clean = " << _treeString[1] << endl;

  if (!_bigValidator->validTree(t1) || !_bigValidator->validTree(t2))
  {
    //delete t1 and t2;
    _bigManager->deleteLast();
    _bigManager->deleteLast();
    cerr << "Error:  Input tree(s) not in proper Unrooted Newick Format"
         << endl;
    return -1;
  }

  // Kernelize
  _bigKernelizor->setT1(t1);
  _bigKernelizor->setT2(t2);
  long flagged = _bigKernelizor->kernelize();
  _numLeavesKernelized = _nameLookup[0].size() - flagged;

  // convert back to strings.  t1 and t2 were flagged in-place
  // and these flags will be skipped when outputting back to strings
  // also, we recompress labels so they range from 0 to k- 1
  // where k is the number of leaves after kernelization
  stringstream ss3;
  ss3 << t1;
  setTree(string(ss3.str()), 0);
  stringstream ss4;
  ss4 << t2;
  setTree(string(ss4.str()), 1);

  // DEBUG
//  cout << "K1 = " << t1 << "\nK2 = " << t2 << endl;
//  cout << "S1 = " << _treeString[0] << "\nS2 = " << _treeString[1] << endl;

  assert(_treeString[0].length() == _treeString[1].length());

  //delete t1 and t2;
  _bigManager->deleteLast();
  _bigManager->deleteLast();
  
  // return the number of deleted leaves
  return flagged;
}

/**
 * Computes the SPR distance of the trees.  
 * @return Distance if success, -1 otherwise.
 */
long TreeDistance::distance(bool useSplitForest)
{
  // check strings
  if (_nameLookup[0] != _nameLookup[1] || _nameLookup[0].empty()
      ||_nameLookup[1].empty())
  {
    cerr << "Error: Input trees have different/empty leaf sets" << endl;
    return -1;
  }

  // make sure all structures are allocated and up to date
  _numTreesSearched = 0;
  _numCacheHits = 0;
  initSearchStructures();

  // Create the Trees
  UPTree<unsigned char> t1, t2;
  _manager->createTree(t1);
  _manager->createTree(t2);

  // check capacity
  if (_nameLookup[0].size() > t1[0].capacity())
  {
    //delete t1 and t2;
    _manager->deleteLast();
    _manager->deleteLast();
    cerr << "Error: Trees are too large" << endl;
    return -1;
  }

  // check size
  if (_nameLookup[0].size() < 4)
  {
    //delete t1 and t2;
    _manager->deleteLast();
    _manager->deleteLast();
    return 0;
  }
    
  // Convert strings and validate
  stringstream ss1;
  ss1 << _treeString[0];
  ss1 >> t1;
  stringstream ss2;
  ss2 << _treeString[1];
  ss2 >> t2;

  if (!_validator->validTree(t1) || !_validator->validTree(t2))
  {
    //delete t1 and t2;
    _manager->deleteLast();
    _manager->deleteLast();
    cerr << "Error:  Input tree(s) not in proper Unrooted Newick Format"
         << endl;
    return -1;
  }

  // longialize the searches.  Each one begins at the opposite tree as the
  // other
  _searcher[0]->setStartTree(t1);
  _searcher[0]->setEndTree(t2);
  _searcher[1]->setStartTree(t2);
  _searcher[1]->setEndTree(t1);

//  cout << "T1 " << t1 << endl;
//  cout << "T2 " << t2 << endl;

  long distance = -1;
  // this is kind of a hack. . should be a nice way to switch
  // between the two types of search (split vs no split).  
  if (useSplitForest == true)
  {
    //delete t1 and t2;
    _manager->deleteLast();
    _manager->deleteLast();
    return splitsForestDistance();
  }
  else
  {
    // iterate the search
    for (size_t i = 1; distance < 0 && i <= _maxIterations; ++i)
    {
      try 
      {
        //DEBUG
        if (useSplitForest == true)
          throw 0;

#ifdef DUAL_SEARCH
        if (_searcher[0]->iterate())
        {
          distance = i * 2 - 1;
        }
        else if (_searcher[1]->iterate())
        {
          distance = i * 2;
        }
#else
        if (_searcher[0]->iterate())
        {
          distance = i;
        }
#endif
/*      cout << "It=" << i << " Checked=" << _cache->_updates 
        << " Dups=" << _cache->_hits  
        << " Colls=" << _cache->_collisions 
        << " CSize=" << _cache->size() 
        << " MSize=" << _manager->size() << endl;
*/
      }
      catch(...)
      {
        return -1;
      }
    }
  }
  _numTreesSearched = _manager->size();
  _numCacheHits = _cache->_hits;

  //delete t1 and t2;
  _manager->deleteLast();
  _manager->deleteLast();
  
  if (distance < 0)
  {
    cerr << "Error: MaxIteration (" << _maxIterations 
         << ") reached before distance found" << endl;
  }
  
  return distance;
}

/**
 * This method expects a supertree to be loaded as t1 and and a reference
 * tree (made up of a subset of the supertree's leafset) to be loaded as
 * t2.  All leaves of the supertree notin the reference tree will be removed
 * so that t1 and t2 are of the same size and comparable by the SPR distance
 * metric.
 * @return 0 is operation was successful.
 */
long TreeDistance::syncLeafSet()
{
   // make sure that some trees ahve been proplerly loaded
  if (_nameLookup[0].empty() || _nameLookup[1].empty())
  {
    cerr << "Error: Input trees have different/empty leaf sets" << endl;
    return -1;
  }

  // make sure that managers are properly allocated
  initLeafSetStructures();
  
  // Create the Trees
  UPTree<unsigned short> t1, t2;
  _bigManager->createTree(t1);
  _bigManager2->createTree(t2);
  string t3;

  // Convert strings and validate
  stringstream ss1;
  ss1 << _treeString[0];
  ss1 >> t1;
  stringstream ss2;
  ss2 << _treeString[1];
  ss2 >> t2;

  if (!_bigValidator->validTree(t1))
  {
    //delete t1 and t2;
    _bigManager->deleteLast();
    _bigManager2->deleteLast();
    delete _bigManager2;
    _bigManager2 = NULL;

    cerr << "Error:  Input tree(s) not in proper Unrooted Newick Format"
         << endl;
    return -1;
  }
  
  bool valid = _bigExtractor.extract(t1, _nameLookup[0], t2, _nameLookup[1], 
                                     t3);
  
  _bigManager->deleteLast();
  _bigManager2->deleteLast();
  delete _bigManager2;
  _bigManager2 = NULL;
  
  if (!valid)
  {
    cerr << "Error:  Unable to extract subtree from supertree" << endl;
    return -1;
  }

  // replace the supertree with the induced subtree.  We are now ready
  // to kernelize and/or find the distance.
  setTree(t3, 0);

  return 0;
}

/**
 *  @return the number of trees in memory.  this is the size of the last
 *          search performed.
 */
size_t TreeDistance::size() const
{
  return _numTreesSearched;
}

size_t TreeDistance::duplicates() const
{
  return _numCacheHits;
}
long TreeDistance::splitsForestDistance()
{
  _numTreesSearched = 0;
  _numCacheHits = 0;
  long distance = 0;
  vector<string> forest1, forest2;
  _splitsForest.extract(_treeString[0], _treeString[1], forest1, forest2);
  
  for (size_t i = 0; i < forest1.size(); ++i)
  {
    setTree(forest1[i], 0);
    setTree(forest2[i], 1);
    kernelize();
    long dist = this->distance(false);
    _numTreesSearched += _manager->size();
    _numCacheHits += _cache->_hits;
    if (dist == -1)
      return -1;
    distance += dist;
  }
  return distance;
}

/**
 * Ensure that all kernelization structures are properly allocated
 * for the current tree size
 */
void TreeDistance::initKernelizeStructures()
{
  if (_bigManager)
    _bigManager->reset();
  
  // ensure that all everything we need to kernelize is properly
  // allocated and initialized
//  if (_bigManager == NULL || _bigManager->numLeaves() != _nameLookup[0].size())
  {
    delete _bigManager;
    delete _bigKernelizor;
    _bigKernelizor = NULL;
    _bigManager = new TreeManager<unsigned short>(_nameLookup[0].size(), 5);
  }
  if (_bigKernelizor == NULL)
  {
    _bigKernelizor = new Kernelizor<unsigned short>(*_bigManager);
  }
  if (_bigValidator == NULL || 
      _bigValidator->getSize() != _bigManager->treeSize())
  {
    delete _bigValidator;
    _bigValidator = new Validator<unsigned short>(_bigManager->treeSize());
  }
}

/**
 * Ensure that all search sutructures are properly allocated for the
 * current tree size.
 */
void TreeDistance::initSearchStructures()
{
  if (_cache)
    _cache->reset();
  if (_manager)
    _manager->reset();

  // ensure that all everything we need to kernelize is properly
  // allocated and initialized
//  if (_manager == NULL || _manager->numLeaves() != _numLeavesKernelized)
  {
    delete _manager;
    delete _cache;
    _manager = new TreeManager<unsigned char>(_numLeavesKernelized, 
                                              _initManagerSize);
    _cache = NULL;
  }
  if (_cache == NULL)
  {
    delete _searcher[0];
    delete _searcher[1];
    _cache = new TreeCache<unsigned char>(_cacheTableSize, _manager);
    _searcher[0] = NULL;
    _searcher[1] = NULL;
  }
  if (_validator == NULL || 
      _validator->getSize() != _manager->treeSize())
  {
    delete _validator;
    _validator = new Validator<unsigned char>(_manager->treeSize());
  }
  if (_searcher[0] == NULL)
  {
    _searcher[0] = new SPRSearch<unsigned char>(*_manager, *_cache, 0);
  }
  if (_searcher[1] == NULL)
  {
    _searcher[1] = new SPRSearch<unsigned char>(*_manager, *_cache, 1);
  }
}

/**
 * Ensure that all structures required for supertree extraction 
 * are properly allocated for the current tree size
 */
void TreeDistance::initLeafSetStructures()
{
  // ensure that all everything we can properly store both the input trees
//  if (_bigManager == NULL || _bigManager->numLeaves() != _nameLookup[0].size())
  {
    delete _bigManager;
    _bigManager = new TreeManager<unsigned short>(_nameLookup[0].size(), 5);
  }

  if (_bigManager2 == NULL || 
      _bigManager2->numLeaves() != _nameLookup[1].size())
  {
    delete _bigManager2;
    _bigManager2 = new TreeManager<unsigned short>(_nameLookup[1].size(), 5);
  }
  if (_bigValidator == NULL || 
      _bigValidator->getSize() != _bigManager->treeSize())
  {
    delete _bigValidator;
    _bigValidator = new Validator<unsigned short>(_bigManager->treeSize());
  }
}
