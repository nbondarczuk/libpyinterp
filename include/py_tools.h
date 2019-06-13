#ifndef _PY_TOOLS_H_
#define _PY_TOOLS_H_

#include "config.h"

#include <python2.7/Python.h>
#include <stdarg.h>

void Py_DecrefAll(int count, ...);

#endif
