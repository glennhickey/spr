#ifndef _FLAGTABLE_H
#define _FLAGTABLE_H

template<class T> class UPTree;
template<class T> class BracketTable;

template<class T> class FlagTable;
template<class T>
std::ostream& operator<<(std::ostream&, const FlagTable<T>&);

/**
 * This class keeps track of a boolean flag for each element of a tree
 * string.  Tt also allows immediate access to the next unflagged
 * neighbour in the string (on the left or right) side.  
 * Note that flagging a leaf also flags a corresponding bracket pair.
 * This table is used in conjunction with the cherry table to perform
 * Rule 1 in linear time. 
 *
 * @author Glenn Hickey
 */
template<class T>
class FlagTable
{
  friend std::ostream& operator<< <T>(std::ostream&, const FlagTable<T>&);

 public:

/**  
  * Flag can specify if a given node is marked as frozen and/or
  * marked for deletion.  The two flags are required as there are
  * some small differences between kernelization and rekernelization
  * Most nodes will be flagged as both except:
  * 1)  An extra Freeze flag is applied to the beginning of common chains
  *     flagged by rule 2.  So if c_3, c_4, ..., c_x are flagged for deletion
  *     then c_2 will also be flagged as freeze-only
  * 2)  All elements, including surrounding brackets, of common subtrees
  *     are flagged for deleltion by rule 1 except for the leaf with the
  *     lowest lable.  For purposes of rekernelization, this leaf is also
  *     flagged as frozen but the surrounding brackets are unflagged. 
  *
  * @author Glenn Hickey
  */
  struct Flag
  {
    unsigned char _del : 1;     /// Marked for deletion (1: marked, 0: not)
    unsigned char _freeze : 1;  /// Marked as frozen (1: marked, 0: not)
    unsigned char _rule : 1;    /// Flagged by which rule (0: 1, 1: 2)
  };

  FlagTable();
  FlagTable(long);
  ~FlagTable();
  void resize(long);
  void setAll(bool, bool);

  long loadTree(const UPTree<T>&, const BracketTable<T>&);
  void copy(const FlagTable<T>&, const long*);

  long leftPos(long) const;
  long rightPos(long) const;
  long cherryPos(long) const;
  void flagLeaf(long);
  void setFlag(long, Flag);
  void setFrozen(long, bool);
  void setDel(long, bool);
  bool isFrozen(long) const;
  bool isDel(long) const;
  Flag flag(long) const;
  Flag operator[](long) const;
  void nestPos(long, long[2]) const;
  void setRuleState(unsigned char);
  void updateRekern();
  long getRightPos(long);
  
 protected:

  void flagUpdateNeighbours(long);
  
  /// Offset the next unflagged neighbour to the left and right of 
  /// a position.  This keeps cherry lookups in the flagged tree O(1).
  struct Neighbours {
    long _left;
    long _right;
  };

  long _size;              /// Size of the table
  Neighbours* _neis;      /// Table of left and right neighbour positions
  Flag* _flags;           /// Table of flags
  const UPTree<T>* _tree; /// Corresponding tree
  const BracketTable<T>* _btable; /// Brackettable of _tree
  long _triPos[3];         /// Position of trifurcation subtrees in FLAGGED tree
  unsigned char _ruleState;  /// Specify if table is flagging in rule 1 or
                             /// rule 2 mode.  
};

#include "flagtable_impl.h"

#endif
