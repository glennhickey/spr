#ifndef _SPLITTABLE_H
#define _SPLITTABLE_H

#include "cvector.h"

/**
 * A split corresponds to the bipartition of the leaf set induced by an
 * longernal edge.
 *
 * @author Glenn Hickey
 */
template<class T> 
class Split : public CVector<T>
{
 public:
  Split();
  Split(size_t);
  size_t _rp;  /// starting polong of right partition
  bool operator<(const Split&) const;
};
template<class T>
std::ostream& operator<<(std::ostream&, const Split<T>&);

/**
 * Split polonger comparison
 *
 * @author Glenn Hickey
 */
template<class T>
struct SplitPLess 
{
  bool operator()(Split<T>*, Split<T>*) ;
};

/**
 * The split table can load all non-trivial splits of a tree and provide
 * set-like operations with split tables of other trees in order to 
 * identify common splits.  This is a quadratic-sized datastructure with 
 * respect to leaf number. 
 *
 * @author Glenn Hickey
 */
template<class T>
class SplitTable
{
 public:
  SplitTable();
  SplitTable(size_t);
  ~SplitTable();
  void resize(size_t);
  long getSize() const;
  const Split<T>& operator[](size_t) const;
  void loadTree(const UPTree<T>&, const BracketTable<T>&);

  const Split<T>* lcs(const SplitTable<T>&) const;
  const long longersection(const SplitTable<T>&, SplitTable<T>&) const;

 protected:
  
  CVector<Split<T>*> _splits;
  Split<T> _temp;
};

template<class T>
std::ostream& operator<<(std::ostream&, const SplitTable<T>&);

#include "splittable_impl.h"

#endif
