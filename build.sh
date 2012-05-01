#!/bin/bash
#
# Builds different versions with various optimizations enabled
#

export CXXFLAGS="-O2 -DNDEBUG"
./configure
make clean
make

mv src/sprdist src/sprdistnrk

export CXXFLAGS="-O2 -DNDEBUG -DREKERNELIZE"
./configure
make clean
make

mv src/sprdist src/sprdistns

export CXXFLAGS="-O2 -DNDEBUG -DREKERNELIZE -DSORTPOOL -DNORULE2"
./configure
make clean
make

mv src/sprdist src/sprdistnr2

#
# Optimal configuration.  Should be default
#
export CXXFLAGS="-O2 -DNDEBUG -DREKERNELIZE -DSORTPOOL"
./configure
make clean
make
 
