#include <sstream>
#include <algorithm>
#include <sstream>

template<class T, class U>
ExtractSubtree<T, U>::ExtractSubtree() 
{

}

template<class T, class U>
ExtractSubtree<T, U>::~ExtractSubtree() 
{

}

/**
 * Removes all leaves from supertree that are not present in reftree
 * and copies the results longo otree.  Note that each leaf removal 
 * is accompanied by the corresponding edge contraction.  It assumed
 * that input trees are numbered from 0 to numleaves-1.  The lookup
 * tables are used to access original values.
 * @param superTree The supertree to extract from
 * @param superLookup Name map for supertree
 * @param refTree The tree whose leafset is to be extracted
 * @param refLookup Name map for reference tree
 * @param otree The output tree in string format
 * @return true if subtree of supertree with leaf set of reftree is 
 * successfully extracted.
 */
template<class T, class U>
bool ExtractSubtree<T, U>::extract(const UPTree<U>& superTree, 
                             const std::vector<std::string>& superLookup,
                             const UPTree<T>& refTree, 
                             const std::vector<std::string>& refLookup,
                             std::string& otree)
{
  _ftable.resize(superTree.size());
  _btable.resize(superTree.size());
  _btable.loadTree(superTree);
  _ftable.loadTree(superTree, _btable);
  _set.clear();

  long fcount = 0;
  long i;
  for (i = 0; i < static_cast<long>(refLookup.size()); ++i)
  {
    _set.insert(refLookup[i]);
  }
  
  for (i = 0; i < superTree.size(); ++i)
  {
    if (superTree[i].myType() == LEAF)
    {
      std::stringstream ss;
      ss << superLookup[superTree[i].myLabel()];
      if (_set.find(ss.str()) == _set.end())
      {
        assert(superTree._man->numLeaves() - fcount > 3);
        _ftable.flagLeaf(i);
        ++fcount;
      }
    }
  }
  
  for (i = 0; i < superTree.size(); ++i)
  {
    if (_ftable.isDel(i) == false)
    {
      if (i > 0 && otree[otree.length() - 1] != '(' && 
          superTree[i].myType() != RB)
        otree +=  ',';
      std::stringstream ss;
      if (superTree[i].myType() == LEAF)
        ss << superLookup[superTree[i].myLabel()];
      else
        ss << superTree[i];
      otree += ss.str();
    }
  }

  return superTree._man->numLeaves() - fcount == refTree._man->numLeaves();
}
