
//
//  Common header file.
//
//  Copyright (c) 2005 Waync Cheng.
//  All Rights Reserved.
//
//  2005/02/21 Waync created.
//

///
/// \file
/// \brief Common header file.
/// \author Waync Cheng
/// \date 2005/02/21
///

#pragma once

//
// Only support C++.
//

#if !defined(__cplusplus)
# error smallworld library requires c++ complier
#endif

//
// STD.
//

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>

namespace sw2 {

//
// Basic type.
//

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

#if defined(WIN32) && defined(_WIN64)
  typedef __int64 int64;
  typedef unsigned __int64 uint64;
#else
  typedef long long int64;
  typedef unsigned long long uint64;
#endif

typedef int64 int_ptr;
typedef uint64 uint_ptr;

} // namespace sw2

#include "swTraceTool.h"

//////////////////////////////////////////////////////////////////////////

///
/// \mainpage Smallworld 2.0 Library
///
/// <img src="../graph.gif">
///
/// Copyright (c) 2005 Waync Cheng.
/// All Rights Reserved.
///

// end of swinc.h
