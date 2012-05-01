#ifdef SORTPOOL
#ifndef REKERNELIZE
#error SORTPOOL MUST BE DEFINED IN CONJUNTION WITH REKERNELIZE
#endif
#endif 

#include "treedistance.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sys/timeb.h>

using namespace std;

int main(long argc, char** argv)
{
  if (argc != 3 && argc != 4)
  {
    cerr << "USAGE: " << argv[0] << " <treefile1> <treefile2> [-s]\n";
    return -1;
  }
  
  bool useSplits = (argc == 4 && string(argv[3]) == "-s");

  ifstream t1file(argv[1]);
  ifstream t2file(argv[2]);
  if (!t1file || !t2file)
  {
    cerr << "Input file error\n";
    return -1;
  }

  string t1, t2;
  TreeDistance tdist;

  t1file >> t1;
  t2file >> t2;
  
  tdist.setTree(t1, 0);
  tdist.setTree(t2, 1);
  tdist.syncLeafSet();
      
  timeb startTime, endTime;
  ftime(&startTime);

  tdist.kernelize();
  long dist = tdist.distance(useSplits);
  size_t count = tdist.size();
  size_t hits = tdist.duplicates();

  ftime(&endTime);
  cout << (useSplits ? "ds: " : "d: ") << dist;
  cout << (useSplits ? " cs: " : " c: ") << count;
  cout << (useSplits ? " hs: " : " h: ") << hits;
  cout << (useSplits ? " ts: " : " t: ") 
       << (float)(endTime.time - startTime.time) +
    (float)((endTime.millitm - startTime.millitm) / 1000.) << "s" << flush;

  return dist;
}
