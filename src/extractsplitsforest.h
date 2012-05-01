#ifndef _EXTRACTSPLITSFOREST_H
#define _EXTRACTSPLITSFOREST_H

#include <vector>
#include <string>
#include <set>
#include "brackettable.h"
#include "flagtable.h"

/**
 * A split corresponds to the bipartition of the leaf set induced by an
 * longernal edge.
 *
 * @author Glenn Hickey
 */
class Split : public std::vector<std::string>
{
 public:
  Split();
  Split(size_t);
  size_t _rp;  /// starting polong of right partition
  long _left, _right;  /// position of split in original tree
  bool _flip;        /// whether or not split was flipped
  bool operator<(const Split&) const;
};

std::ostream& operator<<(std::ostream&, const Split&);

/**
 * Split polonger comparison
 *
 * @author Glenn Hickey
 */
struct SplitPLess 
{
  bool operator()(const Split*, const Split*) const;
};

/*
 * This class will break apart trees at common splits.  
 *
 * @author Glenn Hickey
 */
class ExtractSplitsForest
{
 public:
  
  ExtractSplitsForest();
  ~ExtractSplitsForest();
  
  long extract(const std::string&, const std::string&, 
              std::vector<std::string>&, 
              std::vector<std::string>&);

 protected:

  long getSplits(const std::vector<std::string>&, 
                std::set<Split*, SplitPLess>&);
  bool breakLCS(const std::vector<std::string>&,
                const std::vector<std::string>& tree2,
                std::vector<std::string>&, std::vector<std::string>&,
                std::vector<std::string>&, std::vector<std::string>&);
  void splitTree(const std::vector<std::string>&, const Split&,
                 std::vector<std::string>&, std::vector<std::string>&);
    

  std::vector<std::string> _temp;
  long _nextLabel;
};


#endif
