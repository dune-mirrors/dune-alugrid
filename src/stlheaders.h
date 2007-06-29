#ifndef __ALUGRID_SYSTEMINCLUDES_HH__
#define __ALUGRID_SYSTEMINCLUDES_HH__

// here we include all system header files, otherwise the we have to 
// put the ALUGridSpace namespace in every file of the original
// source and we dont want to do that. so the idea is first to include 
// the system header and then include the original BernhardSchuppGrid
// sources within the namespace defined in bsinclude.hh

#include <ulimit.h>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <cmath>
#include <limits.h>
#include <time.h>
// headers for getrusage(2)
#include <sys/resource.h>

#include <rpc/rpc.h>

#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <functional>
#include <algorithm>
#include <list>
#include <iomanip>
#include <sstream>
#include <deque>
#include <bits/stl_queue.h>
#include <stack>
#include <map>
#include <numeric>
#include <set>

#endif
