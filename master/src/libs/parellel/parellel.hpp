#pragma once

#define USE_TBB

#ifdef USE_TBB
#include "tbb.h"
#endif


#if defined(USE_TBB)
#define USE_PARELLEL
#endif