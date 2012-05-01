#ifndef _KERNELIZOR_H
#define _KERNELIZOR_H

#include <queue>
#include <iostream>

#include "uptree.h"
#include "brackettable.h"
#include "cherrytable.h"
#include "chaintable.h"
#include "flagtable.h"

template<class T> class Kernelizor;

template<class T>
std::ostream& operator<<(std::ostream&, Kernelizor<T>&);

/**
 * Kernelizes pairs of trees by flagging common subtrees and chains in 
 * accordance with Rule 1 and Rule 2.  The time complexity is linear. 
 *
 * @author Glenn Hickey
 */
template<class T>
class Kernelizor 
{
  friend std::ostream& operator<< <T>(std::ostream&, Kernelizor<T>&);

 public:
  
  Kernelizor(TreeManager<T>&);
  ~Kernelizor();
  void setT1(UPTree<T>&);
  void setT2(UPTree<T>&);
  long kernelize();
  size_t numLeaves() const;
  const BracketTable<T>& getBTable(long) const;
  const FlagTable<T>& getFTable(long) const;
  const long* getLookup(long) const;
  
 protected:
    
  void loadTree(UPTree<T>&, long);
  long rule1();
  long rule2();
  long chainLinks(T, T[2]);
  long flagChain(T, T);
  void copyToTemp();
  void copyFromTemp();
  
  UPTree<T> _t[4];       /// _t[0] & _t[1] are the input trees
                         /// _t[2] & _t[3] are the above after Rule1
  TreeManager<T>* _man;  /// tree manager to create _t[2] and _t[3]
  size_t _numLeaves;        /// number of leaves in input trees
  long _tempSize;         /// number of leaves after Rule1
  long* _lookup[4];       /// map label to position in all 4 trees
  BracketTable<T> _btable[4];     /// bracket information tables
  FlagTable<T> _ftable[4];        /// flag tables
  CherryTable<T> _cherryTable[4]; /// cherry tables for Rule 1
  ChainTable<T> _chainTable[4];   /// chain tables for Rule 2
  std::queue<long> _queue; /// queue used in rule 1

 private:
  Kernelizor();
};


#include "kernelizor_impl.h"

#endif
