#ifndef _CXX_COMPATIBILITY_H_
#define _CXX_COMPATIBILITY_H_

#if __cplusplus < 199711L
#include "auto_ptr.h"
#endif

#if __cplusplus < 201103L
#define UNIQUE_PTR auto_ptr
#else
#define UNIQUE_PTR unique_ptr
#endif

#if __cplusplus < 201103L
#define MOVE(X) ::std::move(X)
#else
#define MOVE(X) X
#endif

#endif /* _CXX_COMPATIBILITY_H_ */
