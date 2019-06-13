#ifndef _PY_ERROR_H_
#define _PY_ERROR_H_

#include <string>

#include <python2.7/Python.h>

#define MAX_ERROR_MSG_LEN 256

std::string sys_error_info(int rc, const std::string& msg);
std::string error_info(const std::string& msg);

void Py_Error(std::string& error_message);

#endif
