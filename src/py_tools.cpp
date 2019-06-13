#include "config.h"

#include <python2.7/Python.h>
#include <stdarg.h>

#include "py_tools.h"

void Py_DecrefAll(int count, ...)
{
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        PyObject* ptr = va_arg(args, PyObject*);
        Py_XDECREF(ptr);
    }
    va_end(args);
}
