# searches for STL Headers need to compile the code

AC_DEFUN([ALU3D_CHECK_HEADERS],[ AC_REQUIRE([AC_PROG_CXX])
  AC_LANG_PUSH([C++])
  AC_CHECK_HEADERS(ulimit.h)
  AC_CHECK_HEADERS(assert.h)
  AC_CHECK_HEADERS(stdio.h)
  AC_CHECK_HEADERS(malloc.h)
  AC_CHECK_HEADERS(math.h)
  AC_CHECK_HEADERS(limits.h)
  AC_CHECK_HEADERS(time.h)
  AC_CHECK_HEADERS(memory)
  AC_CHECK_HEADERS(iostream)
  AC_CHECK_HEADERS(fstream)
  AC_CHECK_HEADERS(vector)
  AC_CHECK_HEADERS(utility)
  AC_CHECK_HEADERS(functional)
  AC_CHECK_HEADERS(algorithm)
  AC_CHECK_HEADERS(list)
  AC_CHECK_HEADERS(iomanip)
  AC_CHECK_HEADERS(strstream)
  AC_CHECK_HEADERS(deque)
  AC_CHECK_HEADERS(stack)
  AC_CHECK_HEADERS(map)
  AC_CHECK_HEADERS(numeric)
  AC_CHECK_HEADERS(rpc/rpc.h)
  AC_LANG_POP
])

