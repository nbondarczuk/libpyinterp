#ifndef _PY_INTERPRETER_POOL_H_
#define _PY_INTERPRETER_POOL_H_

#include <deque>
#include <map>
#include <string>
#include <vector>

#include "config.h"

#include <python2.7/Python.h>
#include <pthread.h>

#include "cxx_compatibility.h"

#define DEFAULT_POOL_SIZE 50
#define MAX_TIMEOUT_NS 10000

/*
  Visible types of objects handled by class methods
*/
typedef pthread_mutex_t* PthreadMutexPtr;
typedef pthread_cond_t* PthreadCondPtr;
typedef PyThreadState* PyInterpreterThreadStatePtr;
typedef PyThreadState* PyThreadStatePtr;
typedef PyObject* PyDataHandlerPtr;

/*
  The calss manages a pool of python interpreters. The pool contains a
  queue of initialized interpreter conxtexts. A client may take an
  interpreter from free ones, use it and then return it to the free
  pool. Booked interpreters are stored in a special queue. The context
  of the interpreter contains thread state and the handle open
  for this interpreter in this thread state.
*/
class PyInterpreterPool
{
public:
    // functions
    PyInterpreterPool(int n = DEFAULT_POOL_SIZE);
    ~PyInterpreterPool();
    void start(const std::string& mn, const std::string& dhn);
    size_t size() const;
    PyInterpreterThreadStatePtr alloc(unsigned int max_timeout_ns = MAX_TIMEOUT_NS);
    PyInterpreterThreadStatePtr alloc(PyDataHandlerPtr& handler, unsigned int max_timeout_ns = MAX_TIMEOUT_NS);
    void dealloc(PyInterpreterThreadStatePtr interpreter);
    PyDataHandlerPtr get_handler(PyInterpreterThreadStatePtr interpreter);

private:
    // types
    typedef std::deque<PyInterpreterThreadStatePtr> PyInterpreterThreadStatePtrQueue;
    typedef PyInterpreterThreadStatePtrQueue::iterator PyInterpreterThreadStatePtrQueueIterator;
    typedef PyInterpreterThreadStatePtrQueue::const_iterator PyInterpreterThreadStatePtrQueueConstIterator;
    typedef std::vector<PyDataHandlerPtr> PyDataHandlers;
    typedef std::map<PyInterpreterThreadStatePtr, PyDataHandlerPtr> PyInterpreterThreadStatePtrToDataHandlerPtrMap;
    typedef PyInterpreterThreadStatePtrToDataHandlerPtrMap::iterator PyInterpreterThreadStatePtrToDataHandlerPtrMapIterator;
    typedef PyInterpreterThreadStatePtrToDataHandlerPtrMap::const_iterator PyInterpreterThreadStatePtrToDataHandlerPtrMapConstIterator;
    // members
    static unsigned int global_pools_no;
    const unsigned int pool_size;
    const PthreadMutexPtr mutex;
    const PthreadCondPtr not_empty_free_cond;
    std::string module_name;
    std::string data_handler_name;
    PyInterpreterThreadStatePtrQueue free;
    PyInterpreterThreadStatePtrQueue busy;
    PyInterpreterThreadStatePtrToDataHandlerPtrMap handler;
    // functions
    PthreadMutexPtr make_mutex() const throw();
    PthreadCondPtr make_cond() const throw();
    void init_mt_layer();
    void init_python();
    void init_interpreters();
    void clean_interpreters();
    void clean_mt_layer();
    void clean_python();
    void invariant() const;
    void build_handlers(const std::string& mn,
                        const std::string& dhn);
    void build_handler(PyInterpreterThreadStatePtr interpreter,
                       const std::string& mn,
                       const std::string& dhn);
    PyInterpreterThreadStatePtr move(PyInterpreterThreadStatePtr interpreter,
                                     PyInterpreterThreadStatePtrQueue& src,
                                     PyInterpreterThreadStatePtrQueue& des);
};

#endif /* _PY_INTERPRETER_POOL_H_ */
