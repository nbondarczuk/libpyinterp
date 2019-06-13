#ifndef _PY_INTERPRETER_POOL_GUARD_H_
#define _PY_INTERPRETER_POOL_GUARD_H_

#include "config.h"

#include <python2.7/Python.h>
#include <pthread.h>

#include "lock_guard.h"
#include "py_error.h"
#include "py_interpreter_pool.h"
#include "trace.h"

//
// The type to be used like a context manager, RAII style. It allocates a new
// interpreter and upon destruction (in a context) it is returned to
// the pool. The data of the context may used freely in the current block.
//
struct PyInterpreterPoolGuard
{
    PyInterpreterPoolGuard(PyInterpreterPool& p): pool(p) {
        FRAME;

        interpreter = pool.alloc(handler);
        INFO("Allocated interpreter: " + lexical_cast<string>(interpreter));
        INFO("GIL acquire");
        gstate = PyGILState_Ensure();
        INFO("GIL acquired");
        root = PyThreadState_Get();
        INFO("Saved main thread state: " + lexical_cast<string>(root));
        root = PyThreadState_Swap(interpreter);
        INFO("Python thread swap done to: " + lexical_cast<string>(interpreter));
    }

    ~PyInterpreterPoolGuard() {
        FRAME;

        PyThreadState_Swap(root);
        INFO("Python thread swap done to main thread state: " + lexical_cast<string>(root));
        pool.dealloc(interpreter);
        INFO("Deallocated interpreter done: " + lexical_cast<string>(interpreter));
        INFO("GIL release");
        PyGILState_Release(gstate);
        INFO("GIL released");
    }

    PyObject* operator()(PyObject* args) {
        FRAME;

        INFO("Calling handler: " + lexical_cast<string>(handler));
        PyObject* result = PyObject_CallObject(handler, args);
        if (!result || PyErr_Occurred()) {
            std::string error_message("PyObject_CallObject");
            Py_Error(error_message);
            throw std::runtime_error(error_info(error_message));
        }

        return result;
    }

    PyInterpreterPool& pool;
    PyInterpreterThreadStatePtr interpreter;
    PyDataHandlerPtr handler;
    PyThreadStatePtr root;
    PyGILState_STATE gstate;
};

#endif /* _PY_INTERPRETER_POOL_GUARD_H_ */
