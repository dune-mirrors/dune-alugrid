#ifndef __ALUGRID_SYSTEMINCLUDES_HH__
#define __ALUGRID_SYSTEMINCLUDES_HH__

// here we include all system header files, otherwise the we have to 
// put the ALUGridSpace namespace in every file of the original
// source and we dont want to do that. so the idea is first to include 
// the system header and then include the original BernhardSchuppGrid
// sources within the namespace defined in bsinclude.hh

#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <climits>
#include <ctime>
#include <ulimit.h>

// headers for getrusage(2)
#include <sys/resource.h>
#include <sys/types.h>

// header for XDR 
#include <rpc/rpc.h>
#include <rpc/xdr.h>
#include <endian.h>

#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <functional>
#include <iterator>
#include <algorithm>
#include <list>
#include <iomanip>
#include <sstream>
#include <deque>
#include <queue>
#include <stack>
#include <map>
#include <numeric>
#include <limits>
#include <set>

#endif
