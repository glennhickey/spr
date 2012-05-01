#ifndef _TREECACHE_IMPL_H
#define _TREECACHE_IMPL_H

#include "uptree.h"
#include "treemanager.h"
#include "crc32.h"

static const size_t MAX_NODE_OS = 134217728 - 1; // 2^27 - 1

/**
 * Create the cache
 * @param n Size of the table
 */
template<class T>
TreeCache<T>::TreeCache(size_t n, TreeManager<T>* man) : _man(man), 
  _tabSize(n), _size(0), _hits(0), _updates(0), _collisions(0)
{
  _tab = (Node*)malloc(_tabSize * sizeof(Node));
  for (size_t i = 0; i < _tabSize; ++i)
  {
    _tab[i]._os = MAX_NODE_OS;
    _tab[i]._next = NULL;
  }
}

template<class T>
TreeCache<T>::~TreeCache()
{
  Node* n;
  for (size_t i = 0; i < _tabSize; ++i)
  {
    if (_tab[i]._os >= 0)
    {
      while (_tab[i]._next)
      {
        n = _tab[i]._next;
        _tab[i]._next = _tab[i]._next->_next;
        free(n);
      }
    }
  }
  free(_tab);
}

/**
 * Remove all elements from the table and reset statistics.  
 */
template<class T>
void TreeCache<T>::reset()
{
  Node* n;
  for (size_t i = 0; i < _tabSize; ++i)
  {
    _tab[i]._os = MAX_NODE_OS;
    while (_tab[i]._next != NULL)
    {
      n = _tab[i]._next;
      _tab[i]._next = n->_next;
      free(n);
    }
  }
   _size = 0;
   _hits = 0;
   _updates = 0;
   _collisions = 0;
}

/**
 * Returns the number of elements in the table
 */
template<class T>
inline size_t TreeCache<T>::size() const {
  return _size;
}

/**
 * If the tree exists in the cache, return its flag information. Otherwise
 * insert it.  
 */ 
template<class T>
inline typename TreeCache<T>::Result  TreeCache<T>::update(
  const UPTree<T>& tree, size_t flag, size_t iter)
{
  assert(_man == tree._man);
  return update(static_cast<size_t>(tree._os), flag, iter);
}

/**
 * If the tree exists in the cache, return its flag information. Otherwise
 * insert it.  
 */ 
template<class T>
typename TreeCache<T>::Result TreeCache<T>::update(size_t os, 
                                                   size_t flag, 
                                                   size_t iter)
{
  Result res;
  res._hit = 0;
  unsigned long idx = hash(os);
  ++_updates;

  // Non-empty node
  if (_tab[idx]._os != MAX_NODE_OS)
  {
    if (_man->compare(os, _tab[idx]._os) == true)
    {
      res._hit = 1;
      res._it = _tab[idx]._it;
      res._flag = _tab[idx]._flag;
      ++_hits;
    }
    else
    {
      for (Node* n = _tab[idx]._next; n; n = n->_next)
      {
        if (_man->compare(os, n->_os))
        {
          res._hit = 1;
          res._it = n->_it;
          res._flag = n->_flag;
          ++_hits;
          break;
        }
      }
    }
    if (res._hit == 0)
    {
      ++_collisions;
      Node* newNode = (Node*)malloc(sizeof(Node));
      *newNode = _tab[idx];
      _tab[idx]._os = os;
      _tab[idx]._flag = flag;
      _tab[idx]._it = iter;
      _tab[idx]._next = newNode;
      ++_size;
    }
  }
  // empty node
  else
  {
    _tab[idx]._os = os;
    _tab[idx]._flag = flag;
    _tab[idx]._it = iter;
    _tab[idx]._next = NULL;
    ++_size;
  }
  return res;
}

/**
 * Hash a tree longo an size_t.  This is a dumb placeholder function
 * that needs to be replaced.  I am currently thinking of crc-32 or a 
 * variant.
 */
template<class T>
inline unsigned long TreeCache<T>::hash(size_t os)
{
  UPTree<T> tree = {os, _man};
  return Crc32<T>::getInstance()->crc(tree) % _tabSize;
}

#endif
