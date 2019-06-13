#include <string>

#include "config.h"

#include <python2.7/Python.h>
#include <pthread.h>

#include "gtest/gtest.h"
#include "lexical_cast.h"
#include "lock_guard.h"
#include "py_error.h"
#include "py_tools.h"
#include "py_interpreter_pool.h"
#include "py_interpreter_pool_guard.h"
#include "strutl.h"

using namespace std;

class interpreter_pool_fixture: public testing::Test
{
public:
    PyInterpreterPool ip;
    unsigned int pool_size;

    interpreter_pool_fixture(): pool_size(50) {}

    void SetUp() {
        ip.start("string", "upper");
    }

    void TearDown() {}
};

TEST_F(interpreter_pool_fixture, testPoolSize)
{
    ASSERT_EQ(pool_size, ip.size());
}

TEST_F(interpreter_pool_fixture, testPoolCallAll)
{
    for (unsigned int i = 0; i < ip.size(); ++i) {
        PyInterpreterPoolGuard ipg(ip);
        string value("abc");
        PyObject* arg = Py_BuildValue("s", value.c_str());
        PyObject* argv = PyTuple_Pack(1, arg);
        PyObject* rv = ipg(argv);
        string reply(PyString_AsString(rv));
        ASSERT_EQ(reply, "ABC");
    }
}

TEST_F(interpreter_pool_fixture, testPoolNew)
{
    PyInterpreterPool* ip2 = new PyInterpreterPool(pool_size * 2);
    ip2->start("string", "lower");
    ASSERT_EQ(pool_size * 2, ip2->size());
    delete ip2;
}

TEST_F(interpreter_pool_fixture, testPoolNewLocalRun)
{
    PyInterpreterPool* ip3 = new PyInterpreterPool();
    ip3->start("string", "lower");
    for (unsigned int i = 0; i < ip3->size(); ++i) {
        PyInterpreterPoolGuard ipg(*ip3);
        string value("ABC");
        PyObject* arg = Py_BuildValue("s", value.c_str());
        PyObject* argv = PyTuple_Pack(1, arg);
        PyObject* rv = ipg(argv);
        string reply(PyString_AsString(rv));
        ASSERT_EQ(reply, "abc");
    }

    delete ip3;
}
