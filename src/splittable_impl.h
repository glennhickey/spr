#include <set>

template<class T>
Split<T>::Split() : CVector<T>()
{
}

template<class T>
Split<T>::Split(size_t size) : CVector<T>(size)
{
}

template<class T>
bool Split<T>::operator<(const Split<T>& split) const
{
  if (_rp < split._rp)
    return true;
  if (_rp > split._rp)
    return false;
  for (size_t i = 0; i < _rp; ++i)
  {
    if (_array[i] < split._array[i])
      return true;
    if (_array[i] > split._array[i])
      return false;
  }
  return false;
}

template<class T>
bool SplitPLess<T>::operator()(Split<T>* s1, Split<T>* s2)
{
  return *s1 < *s2;
}

template<class T>
std::ostream& operator<<(std::ostream& os, const Split<T>& split)
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
SplitTable<T>::SplitTable()
{
}

template<class T>
SplitTable<T>::SplitTable(size_t size) : _splits(size)
{
  for (size_t i = 0; i < size; ++i)
    _splits[i] = NULL;
}

template<class T>
SplitTable<T>::~SplitTable()
{
  for (size_t i = 0; i < _splits.size(); ++i)
    delete _splits[i];
}

template<class T>
void SplitTable<T>::resize(size_t numLeaves)
{
  size_t oldN = std::max(0, (long)_splits.size() - 3);
  size_t i;

  for (i = numLeaves - 3; i < oldN; ++i)
    delete _splits[i];

  _splits.setSize(numLeaves - 3);
  _splits.reserve(numLeaves - 2);

  for (i = 0; i < oldN; ++i)
    _splits[i]->reserve(numLeaves + 1);

  for (i = oldN; i < _splits.size(); ++i)
    _splits[i] = new Split<T>(numLeaves + 1);
}

template<class T>
long SplitTable<T>::getSize() const
{
  return _splits.size();
}

template<class T>
const Split<T>& SplitTable<T>::operator[](size_t i) const
{
  return *_splits[i];
}

template<class T> 
void SplitTable<T>::loadTree(const UPTree<T>& tree, 
                             const BracketTable<T>& btable)
{
  resize(tree._man->numLeaves());

  size_t i, j;
  size_t k = 0;

  for (i = 1; i < static_cast<size_t>(tree.size()); ++i)
  {
    if (tree[i].myType() == LB)
    {
      _splits[k]->clear();
      for (j = 0; j < i; ++j)
      {
        if (tree[j].myType() == LEAF)
        {
          _splits[k]->push_back(tree[j].myLabel());
        }
      }
      for (j = btable[i].pos[1] + 1; j < tree.size(); ++j)
      {
                if (tree[j].myType() == LEAF)
        {
          _splits[k]->push_back(tree[j].myLabel());
        }
      }
      _splits[k]->_rp = _splits[k]->size();
      for (j = i + 1; j < btable[i].pos[1]; ++j)
      {
        if (tree[j].myType() == LEAF)
        {
          _splits[k]->push_back(tree[j].myLabel());
        }
      }

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
    }
  }

  std::sort(&_splits[0], &_splits[_splits.size()], SplitPLess<T>());
    
}

template<class T>
const Split<T>* SplitTable<T>::lcs(const SplitTable<T>& table) const
{

  // placeholder.  d'oh!
  std::vector<Split<T>*> s1, s2, s3;
  for (size_t i = 0; i < _splits.size(); ++i)
    s1.push_back(_splits[i]);
  std::sort(s1.begin(), s1.end(), SplitPLess<T>());
  for (size_t i = 0; i < table._splits.size(); ++i)
    s2.push_back(table._splits[i]);
  std::sort(s2.begin(), s2.end(), SplitPLess<T>());
  
//  std::set_longersection(s1.begin(), s1.end(), s2.begin(), s2.end(), s3.begin(),
//                        SplitPLess<T>());

  for (size_t i = 0; i < s1.size(); ++i)
  {
    for (size_t j = 0; j < s2.size(); ++j)
    {
      SplitPLess<T> pl;
      if (pl(s1[i], s2[j]) == false && pl(s2[j], s1[i]) == false)
      {
        s3.push_back(s1[i]);
        std::cout << *s1[i] << std::endl;
      }
    }
//    if (std::binary_search(s2.begin(), s2.end(), s1[i], SplitPLess<T>()))
//      s3.push_back(s1[i]);
  }

  Split<T>* min = NULL;
  size_t mval = _splits.size();
  for (typename std::vector<Split<T>*>::iterator it = s3.begin(); 
       it != s3.end(); ++it)
  {
    if ((long)abs((long)(*it)->_rp - (long)(*it)->size() / 2) < mval)
    {
      min = *it;
      mval = (long)abs((long)(*it)->_rp - (long)(*it)->size() / 2);
    }
  }

  return min;
}

/*template<class T>
long SplitTable<T>::longersection(const SplitTable<T>& itable, 
                                      SplitTable<T>& otable) const
{
  return -1;
}
*/
template<class T>
std::ostream& operator<<(std::ostream& os, const SplitTable<T>& table)
{
  for (long i = 0; i < table.getSize(); ++i)
    os << table[i] << std::endl;
  return os;
}
