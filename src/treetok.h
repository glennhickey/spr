#ifndef _TREETOK_H
#define _TREETOK_H

#include <iostream>
#include <cassert>

/**
 * The three possible types of a tree token.
 */
enum TTYPE {LB = 0, RB, LEAF};

/**
 * Token used in the binary tree representation.  Can be either a Left Bracket
 * (LB), Right Bracket (RB) or Leaf (LEAF).  Note that template parameter T
 * MUST BE UNSIGNED for this class to behave properly.  
 *
 * TODO:  we don't need to store flags in the treetok since 
 * all flags are temporary and can be passed from kernelizor
 * to searcher in a separate structure (ie flagtable)
 * also, we don't need to waste a whole bit for bracket flags
 * more efficient:  ( = num_limits<T>::max() - 1, ) = num_limits<T>::max()
 * this allows up to 2^8 - 2 to be stored in a byte tree as opposed to
 * 2^5 (difference = 253 vs 32)
 *
 * @author Glenn Hickey
 */
template<class T>
struct TreeTok
{
  /// The leaf label if token is a leaf.
  T _val : sizeof(T) * 8 - 1; //3;
  /// Type of token.  0 = LB, 1 = RB, 2 = LEAF
//  T _type : 2;
  /// Boolean flag.  Used to determine if token is flagged for deletion
  T _flag : 1;

  static const T LBVal;
  static const T RBVal;

  TTYPE myType() const;
  T myLabel() const;
  bool myFlag() const;

  void setLB();
  void setRB();
  void setLeaf(T);
  void setFlag(bool);
  void set(char*);
  size_t capacity() const;
};

template<class T>
std::ostream& operator<<(std::ostream&, const TreeTok<T>);

#include "treetok_impl.h"

#endif
