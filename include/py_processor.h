#ifndef __PY_PROCESSOR_H_
#define __PY_PROCESSOR_H_

#include <map>
#include <pthread.h>
#include <string>

#include "config.h"

#include <python2.7/Python.h>

#include "strutl.h"
#include "lexical_cast.h"
#include "py_error.h"
#include "py_tools.h"
#include "py_interpreter_pool.h"
#include "py_interpreter_pool_guard.h"
#include "trace.h"

#define PYTHON_DATA_HANDLER "process_data_logic"

typedef std::map<std::string, std::string> MapString2String;
typedef std::multimap<std::string, std::string> MultimapString2String;
typedef MapString2String::const_iterator MapString2StringConstIterator;
typedef MultimapString2String::const_iterator MultimapString2StringConstIterator;

/* 
 * Python backend processor
*/
class PyProcessor
{
  public:
    PyProcessor(const std::string& processor_module_name);
    ~PyProcessor();
    std::string Process(const std::string& identifier,
                        MapString2String& messages,
                        MultimapString2String& parameters);

  private:
    std::string module_name;
    PyInterpreterPool ip;
    PyObject* map2dict(const MapString2String& messages);
    PyObject* multimap2dict(const MultimapString2String& messages);
};

#endif /* __PY_PROCESSOR_H_ */
