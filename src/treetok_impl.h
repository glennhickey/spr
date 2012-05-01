#ifndef _TREETOK_IMPL_H
#define _TREETOK_IMPL_H

#include <math.h>
/**
 * Return token type.
 * @see enum TTYPE
 */
template<class T>
inline TTYPE TreeTok<T>::myType() const 
{ 
  return _val == RBVal ? RB : _val == LBVal ? LB : LEAF; //(TTYPE)_type;
}

/**
 * Return leaf label.  Only applies to leaf tokens.
 */  
template<class T>
inline T TreeTok<T>::myLabel() const 
{
  assert(myType() == LEAF);
  return _val;
}

/**
 * Return boolean flag.
 */
template<class T>
inline bool TreeTok<T>::myFlag() const 
{
  return static_cast<bool>(_flag);
}

/**
 * Set token to left bracket.  Flag is reset.
 */ 
template<class T>
inline void TreeTok<T>::setLB() 
{
  //_type = LB;
  _val = LBVal;
  _flag = 0;
}

/**
 * Set token to right bracket.  Flag is reset.
 */
template<class T>
inline void TreeTok<T>::setRB() 
{
//  _type = RB;
  _val = RBVal;
  _flag = 0;
}

/**
 * Set token to leaf and assign label value.  Flag is reset.
 */
template<class T>
inline void TreeTok<T>::setLeaf(T label) 
{
  assert(label != LBVal && label != RBVal);
//  _type = LEAF;
  _val = label;
  _flag = 0;
}

/**
 * Set boolean flag of token.
 */
template<class T>
inline void TreeTok<T>::setFlag(bool flag) 
{
  _flag = static_cast<T>(flag);
}

/**
 * Convert character string to a token.  String should be 
 * ")", "(" or a leaf label.
 */
template<class T>
inline void TreeTok<T>::set(char* s) 
{
  if (s[0] == '(')
    setLB();
  else if (s[0] == ')')
    setRB();
  else
    setLeaf(atoi(s));
}

/**
 * Return the maximum number of unique leaf labels that can be stored in
 * a token of this size.
 */
template<class T>
inline size_t TreeTok<T>::capacity() const
{
  return static_cast<long>(pow(2., static_cast<double>(sizeof(T) * 8 - 1))) - 1;
}

/**
 * Prlong token to a stream.  Flag value is not displayed. 
 */
template<class T>
std::ostream& operator<<(std::ostream& os, const TreeTok<T> tok)
{
  TTYPE toktype = tok.myType();
  if (toktype == LB)
    os << "(";
  else if (toktype == RB)
    os << ")";
  else 
    os << static_cast<size_t>(tok.myLabel());
  return os;
}

#endif
