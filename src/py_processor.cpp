#include <cstdio>
#include <sstream>

#include "config.h"

#include <python2.7/Python.h>

#include "config.h"
#include "cxx_compatibility.h"
#include "trace.h"
#include "strutl.h"
#include "lexical_cast.h"
#include "py_error.h"
#include "py_tools.h"
#include "py_interpreter_pool.h"
#include "py_interpreter_pool_guard.h"
#include "py_processor.h"

using namespace std;

/*
 * Constructor of python processor starting it on a specific handler
 */
PyProcessor::PyProcessor(const string& processor_module_name): module_name(processor_module_name)
{
    FRAME;
	
    ip.start(module_name, PYTHON_DATA_HANDLER);
    INFO("Started python interpreter(s) "
		 + lexical_cast<string>(ip.size())
		 + " for: "
		 + module_name
		 + "."
		 + PYTHON_DATA_HANDLER);
}

/*
 * Destructor implicitely deallocating the pool of interpreters
 */
PyProcessor::~PyProcessor()
{
    FRAME;

    INFO("Finishing python interpreter(s) "
		 + lexical_cast<string>(ip.size())
		 + " for: "
		 + module_name
		 + "."
		 + PYTHON_DATA_HANDLER);
}

/*
 * Processor allocating next available interpreter and releasin it
 */
string
PyProcessor::Process(const string& identifier,
                         MapString2String& messages,
                         MultimapString2String& parameters)
{
    FRAME;

    PyInterpreterPoolGuard ipg(ip);

    // prepare parameters
    PyObject* py_key = Py_BuildValue("s", identifier.c_str());
    PyObject* py_messages = map2dict(messages);
    PyObject* py_parameters = multimap2dict(parameters);
    PyObject* py_argv = PyTuple_Pack(3, py_key, py_messages, py_parameters);
    if (!py_key || !py_messages || !py_parameters || !py_argv) {
        string error_message("Empty value in python build value");
        throw runtime_error(error_message);
    }

    // call data handler with parametrers
    INFO("Calling guarded python module: " + module_name);
    PyObject* py_result = ipg(py_argv);
    if (!py_result) {
        string error_message("NULL value received from processor");
        throw runtime_error(error_message);
    }
	
    string content = PyString_AsString(py_result);

    Py_DecrefAll(5, py_key, py_messages, py_parameters, py_argv, py_result);
    INFO("Finished guarded python module: "
		 + module_name
		 + " with content:\n"
		 + content);
	
    // get result releasing the guard
    return content;
}

/*
 * Creator of python dict from a map of messages
 */
PyObject*
PyProcessor::map2dict(const MapString2String& messages)
{
    PyObject* pDict = PyDict_New();
    for (MapString2StringConstIterator it = messages.begin();
         it != messages.end();
         ++it) {
        PyObject* value = Py_BuildValue("s", it->second.c_str());
        if (!value) {
            return NULL;
        } else {
            PyDict_SetItemString(pDict, it->first.c_str(), value);
        }
    }

    return pDict;
}

/*
 * Creator of python dict from a multimap of messages
 */
PyObject*
PyProcessor::multimap2dict(const MultimapString2String& messages)
{
    PyObject* pDict = PyDict_New();
    for (MultimapString2StringConstIterator it = messages.begin();
         it != messages.end();
         ++it) {
        PyObject* value = Py_BuildValue("s", it->second.c_str());
        if (!value) {
            return NULL;
        } else {
            PyDict_SetItemString(pDict, it->first.c_str(), value);
        }
    }

    return pDict;
}
