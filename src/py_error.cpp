#include <string>

#include <string.h>

#include "config.h"

#include <python2.7/Python.h>

#include "lexical_cast.h"
#include "py_error.h"

using namespace std;

string sys_error_info(int rc, const string& msg)
{
    char error_buf[MAX_ERROR_MSG_LEN];
    char* error_msg = strerror_r(rc, error_buf, sizeof(error_buf));
    return "System Error: " + msg + ", rc: " + lexical_cast<string>(rc) + ", " + error_msg;
}

string error_info(const string& msg)
{
    return "Error: " + msg;
}

void Py_Error(string& error_message)
{
    PyObject *pExcType, *pExcValue, *pExcTraceback;
    PyErr_Fetch(&pExcType, &pExcValue, &pExcTraceback);
    PyErr_NormalizeException(&pExcType, &pExcValue, &pExcTraceback);

    PyTypeObject* type = (PyTypeObject*)pExcType;
    string error_code(type->tp_name);
    if (0 == error_message.length()) {
        error_message = error_code;
    } else {
        error_message += ": ";
        error_message += error_code;
    }

    error_message += ": ";

    if (pExcType != NULL) {
        Py_DecRef(pExcType);
    }

    if (pExcValue != NULL) {
        PyObject* otext = PyObject_GetAttrString(pExcValue, "message");
        if (otext) {
            error_message += PyString_AsString(otext);
            Py_DecRef(otext);
            Py_DecRef(pExcValue);
        }
    }

    if (pExcTraceback != NULL) {
        Py_DecRef(pExcTraceback);
    }
}
