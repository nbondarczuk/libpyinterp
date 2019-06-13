#ifndef _PY_GIL_GUARD_H_
#define _PY_GIL_GUARD_H_

#include "config.h"

#include <python2.7/Python.h>
#include <pthread.h>

#include "lock_guard.h"
#include "py_error.h"
#include "py_interpreter_pool.h"
#include "trace.h"

struct PyGILGuard
{
    PyGILGuard(bool tbr = true): to_be_released(tbr) {
        FRAME;

        INFO("GIL acquire");
        gstate = PyGILState_Ensure();
        INFO("GIL acquired");
        main_ts = PyThreadState_Get();
        INFO("Saved main thread state: " + lexical_cast<string>(main_ts));
    }

    ~PyGILGuard() {
        FRAME;

        if (to_be_released) {
            PyThreadState_Swap(main_ts);
            INFO("Thread state swap to: " + lexical_cast<string>(main_ts));
            INFO("GIL release");
            PyGILState_Release(gstate);
            INFO("GIL released");
        }
    }

    bool to_be_released;
    PyThreadStatePtr main_ts;
    PyGILState_STATE gstate;
};

#endif /* _PY_GIL_GUARD_H_ */
