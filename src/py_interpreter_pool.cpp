#include <algorithm>
#include <deque>
#include <memory>
#include <new>
#include <string>

#include "config.h"

#include <python2.7/Python.h>
#include <pthread.h>
#include <time.h>

#include "cxx_compatibility.h"
#include "trace.h"
#include "lexical_cast.h"
#include "lock_guard.h"
#include "py_tools.h"
#include "py_error.h"
#include "py_interpreter_pool.h"
#include "py_gil_guard.h"

using namespace std;

unsigned int PyInterpreterPool::global_pools_no = 0;
pthread_mutex_t global_pool_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Heap allocation of a new mutex
 */
PthreadMutexPtr
PyInterpreterPool::make_mutex() const throw()
{
    return new (std::nothrow) pthread_mutex_t;
}

/*
 * Heap allocation of a new condtional variable
 */
PthreadCondPtr
PyInterpreterPool::make_cond() const throw()
{
    return new (std::nothrow) pthread_cond_t;
}

/*
 * Make pool and allocate thread control structures
 */
PyInterpreterPool::PyInterpreterPool(int n):
    pool_size(n),
    mutex(make_mutex()),
    not_empty_free_cond(make_cond())
{
    FRAME;

    LockGuard<pthread_mutex_t> gpm(&global_pool_mutex);

    init_mt_layer();
    try {
        if (0 == global_pools_no++) {
            init_python();
        }

        init_interpreters();
    } catch(exception& e) {
        global_pools_no--;
		INFO("Got exception: " + e.what());
        throw;
    }
}

/*
 * Release all interpreters and clean cuncurrency framework
 */
PyInterpreterPool::~PyInterpreterPool()
{
    FRAME;

    LockGuard<pthread_mutex_t> gpm(&global_pool_mutex);

    try {
        clean_interpreters();
        global_pools_no--;
        if (0 == global_pools_no) {
            clean_python();
        }

    } catch(exception& e) {
		INFO("Got exception: " + e.what());
	}

    clean_mt_layer();
}

/*
 * Create data handler
 */
void PyInterpreterPool::build_handler(PyInterpreterThreadStatePtr interpreter,
                                      const string& mn,
                                      const string& dhn)
{
    FRAME;

    string error_message;
    PyObject* py_module_name = PyString_FromString(mn.c_str());
    if (!py_module_name) {
        error_message = "creating module name: " + mn;
    } else {
        PyObject* py_module = PyImport_Import(py_module_name);
        if (!py_module) {
            error_message = "importing module: " + mn;
        } else {
            PyObject* py_data_handler = PyObject_GetAttrString(py_module, dhn.c_str());
            if (!(py_data_handler && PyCallable_Check(py_data_handler))) {
                error_message = "building data handler";
            } else {
                Py_DecrefAll(2, py_module_name, py_module);
                INFO("Created data handler: " + lexical_cast<string>(py_data_handler));
                handler.insert(pair<PyInterpreterThreadStatePtr, PyDataHandlerPtr>(interpreter, py_data_handler));
                INFO("Loaded data handler map: " +
                     lexical_cast<string>(interpreter) + " -> " +
                     lexical_cast<string>(py_data_handler));
            }
        }
    }

    if (!error_message.empty()) {
        if (PyErr_Occurred() != NULL) {
            Py_Error(error_message);
            throw runtime_error(error_info(error_message));
        }
    }
}

/*
 * Make all handles and link them with interpreter
 */
void PyInterpreterPool::build_handlers(const string& mn,
                                       const string& dhn)
{
    FRAME;

    INFO("Creating handlers for: " + mn + "." + dhn);

    for (unsigned int i = 0; i < pool_size; i++) {
        PyInterpreterThreadStatePtr interpreter = alloc();
        INFO("Got next interpreter: " + lexical_cast<string>(interpreter));
        PyThreadState_Swap(interpreter);
        build_handler(interpreter, mn, dhn);
        PyThreadState_Swap(NULL);
        dealloc(interpreter);
    }

    INFO("Created handlers no: " + lexical_cast<string>(handler.size()));
}

/*
 * Build N interpreters with their contexts: python thread states and
 * data handlers for each of them. Store states and handlers in map.
 * The interpreter may then execut in its own context.
 */
void PyInterpreterPool::start(const string& mn, const string& dhn)
{
    FRAME;

    LockGuard<pthread_mutex_t> gpm(&global_pool_mutex);

    build_handlers(mn, dhn);

    // finally, the object is created
    module_name = mn;
    data_handler_name = dhn;

    invariant(); // must hold since now on
}

/*
 * Get next free interpreter from the queue waiting for the condition:
 * queue not empty. No global mutex used here as this is a condition
 * we wait for: not empty queue of interpreters. The allocated
 * interpreter is returned but it alst is stored on the busy queue
 * so that the do not loose it.
 */
PyInterpreterThreadStatePtr
PyInterpreterPool::alloc(unsigned int max_timeout_ns)
{
    FRAME;

    LockGuard<pthread_mutex_t> m(mutex);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += max_timeout_ns;

    int rc = 0;
    while (free.empty() && rc == 0) {
        rc = pthread_cond_timedwait(not_empty_free_cond, mutex, &ts);
    }

    if (rc != 0) {
        throw runtime_error(sys_error_info(rc, "pthread_cond_wait"));
    }

    PyInterpreterThreadStatePtr interpreter = move(free.front(), free, busy);

    return interpreter;
}

PyInterpreterThreadStatePtr
PyInterpreterPool::alloc(PyDataHandlerPtr& handler_rv,
                         unsigned int max_timeout_ns)
{
    FRAME;

    LockGuard<pthread_mutex_t> m(mutex);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += max_timeout_ns;

    int rc = 0;
    while (free.empty() && rc == 0) {
        rc = pthread_cond_timedwait(not_empty_free_cond, mutex, &ts);
    }

    if (rc != 0) {
        throw runtime_error(sys_error_info(rc, "pthread_cond_wait"));
    }

    PyInterpreterThreadStatePtr interpreter = move(free.front(), free, busy);
    PyInterpreterThreadStatePtrToDataHandlerPtrMapConstIterator it = handler.find(interpreter);
    if (it != handler.end()) {
        handler_rv = handler.find(interpreter)->second;
    } else {
        throw runtime_error(error_info("handler of interpreter missing"));
    }

    return interpreter;
}

/*
 * Put back the interpreter on the queue raising the condition: queue
 * not empty. No global mutex used here as we signal the condition
 * to potentiol waiters: the queue is not empty.
 */
void PyInterpreterPool::dealloc(PyInterpreterThreadStatePtr interpreter)
{
    FRAME;

    LockGuard<pthread_mutex_t> m(mutex);

    (void)move(interpreter, busy, free);
    int rc = pthread_cond_broadcast(not_empty_free_cond);
    if (rc != 0) {
        throw runtime_error(sys_error_info(rc, "pthread_cond_broadcast"));
    }
}

/*
  Get the size of the pool of interpreters. No global mutex used here
  but just the local one: we can't just return the state of the
  queues, free and busy as there can be some pending operation on them.
*/
size_t PyInterpreterPool::size() const
{
    FRAME;

    LockGuard<pthread_mutex_t> m(mutex);

    invariant();

    return free.size() + busy.size();
}

/*
 * Initialize mutex, cond variable checking if they memoery is Ok
 */
void PyInterpreterPool::init_mt_layer()
{
    FRAME;

    if (!mutex) {
        throw runtime_error(error_info("pthread_mutex not allocated"));
    }

    if (!not_empty_free_cond) {
        if (mutex) {
            delete mutex;
        }

        throw runtime_error(error_info("pthread_cond not allocated"));
    }

    int rc = pthread_mutex_init(mutex, NULL);
    if (rc != 0) {
        throw runtime_error(sys_error_info(rc, "pthread_mutex_init"));
    }

    rc = pthread_cond_init(not_empty_free_cond, NULL);
    if (rc != 0) {
        throw runtime_error(sys_error_info(rc, "pthread_cond_init"));
    }
}

/*
 * Only once, init python environment
 */
void PyInterpreterPool::init_python()
{
    FRAME;

    Py_Initialize();
    PyEval_InitThreads();
    PyEval_ReleaseLock();

    INFO("Initialized python with threads");
}

/*
 * Create interpreters in global critical section
 */
void PyInterpreterPool::init_interpreters()
{
    FRAME;

    INFO("Creating interpreters: " + lexical_cast<string>(pool_size));

    for (unsigned int i = 0; i < pool_size; i++) {
        PyInterpreterThreadStatePtr interpreter = Py_NewInterpreter();
        if (!interpreter) {
            if (PyErr_Occurred() != NULL) {
                string error_message("Py_NewInterpreter");
                Py_Error(error_message);
                throw runtime_error(error_info(error_message));
            }
        } else {
            free.push_front(interpreter);
            INFO("Created interpreter Py_NewInterpreter [" +
                 lexical_cast<string>(i) + "] " +
                 lexical_cast<string>(interpreter));
        }
    }

    INFO("Created interpreters: " + lexical_cast<string>(free.size()));
}

/*
  Release all interpreters and destroy synch layer in global citical
  section, The mutex, cond variables being smart pointers will be
  released automaticly so not need to do manually.
*/
void PyInterpreterPool::clean_interpreters()
{
    FRAME;

    PyGILGuard g;

    // first handlers, so that the pointers are not invalidated
    for (PyInterpreterThreadStatePtrToDataHandlerPtrMapIterator it = handler.begin();
         it != handler.end();
         ++it) {
        PyThreadState_Swap(it->first);
        Py_XDECREF(it->second);
        PyThreadState_Swap(NULL);
    }

    // no problem, they are not busy
    for (PyInterpreterThreadStatePtrQueueIterator it = free.begin();
         it != free.end();
         ++it) {
        PyThreadState_Swap(*it);
        Py_EndInterpreter(*it);
        PyThreadState_Swap(NULL);
    }

    // brutal, gracefull ending missing
    for (PyInterpreterThreadStatePtrQueueIterator it = busy.begin();
         it != busy.end();
         ++it) {
        PyThreadState_Swap(*it);
        Py_EndInterpreter(*it);
        PyThreadState_Swap(NULL);
    }
}

void PyInterpreterPool::clean_mt_layer()
{
    FRAME;

    int rc = pthread_mutex_destroy(mutex);
    if (rc != 0) {
        cerr << sys_error_info(rc, "pthread_mutex_destroy") << endl;
    } else {
        delete mutex;
    }

    rc = pthread_cond_destroy(not_empty_free_cond);
    if (rc != 0) {
        cerr << sys_error_info(rc, "pthread_cond_destroy") << endl;
    } else {
        delete not_empty_free_cond;
    }
}

void PyInterpreterPool::clean_python()
{
    FRAME;

    PyGILGuard g(false);

    Py_Finalize();
    // No GIL anuymore, so no matching release, the guard is fake
}

/*
 * Class invariant may not be violated:
 * - number of data handlers is the same as pool size
 * - numer of free and busy inerpreters is same as pool size
 */
void PyInterpreterPool::invariant() const
{
    assert(free.size() + busy.size() == pool_size);
    assert(handler.size() == pool_size);
}

/*
 * Move the interpreter from one queue to another. It has do be done
 * in a scope of a guard, a critical section, while managing free interpreter pool.
 */
PyInterpreterThreadStatePtr
PyInterpreterPool::move(PyInterpreterThreadStatePtr interpreter,
                        PyInterpreterThreadStatePtrQueue& src,
                        PyInterpreterThreadStatePtrQueue& des)
{
    FRAME;

    INFO("Moving interpreter: " + lexical_cast<string>(interpreter));

    PyInterpreterThreadStatePtrQueueIterator it = find(src.begin(), src.end(), interpreter);
    if (it == src.end()) {
        throw logic_error(error_info("Cant find interpreter in queue"));
    } else {
        src.erase(it);
        des.push_back(interpreter);
    }

    return interpreter;
}
