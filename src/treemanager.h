#ifndef _TREEMANAGER_H
#define _TREEMANAGER_H

template<class T> struct TreeTok; 
template<class T> struct UPTree;


/**
 * Stores a given number of trees in a contiguous chunk of memory. 
 * Resized with realloc if the current capacity is exceeded.  
 * realloc is used instead of new as it can potentially increase the
 * block without requiring a copy of the entire thing (and uptrees don't
 * have constructors so this isn't a problem). 
 *
 * @author Glenn Hickey
 */
template<class T = short>
class TreeManager
{
  public:
  TreeManager(size_t, size_t);
  ~TreeManager();
  
  TreeTok<T> operator[](size_t) const;
  TreeTok<T>& operator[](size_t);
 
  size_t treeSize() const;
  size_t numLeaves() const;
  size_t size() const;
  void reset();

  void createTree(UPTree<T>&);
  void getTree(size_t, UPTree<T>&); 
  void deleteLast();
  bool compare(size_t, size_t);

  protected:
  TreeTok<T>* _block;        /// Address of memory block
  size_t _treeSize;             /// Size of each tree of block in #tokens
  size_t _size;                 /// # trees currently in manager
  size_t _cap;                  /// capacity in # trees

  private:
  TreeManager();
};

#include "treemanager_impl.h"

#endif
