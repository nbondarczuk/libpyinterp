#include <string>

#include <pthread.h>

#include "gtest/gtest.h"
#include "lexical_cast.h"
#include "lock_guard.h"

using namespace std;

class lock_guard_fixture: public testing::Test
{
public:
    lock_guard_fixture(): m(NULL) {}

    void SetUp() {
        m = new pthread_mutex_t;
        int rc = pthread_mutex_init(m, NULL);
        if (rc != 0) {
            throw runtime_error("Error: pthread_mutex_init(), rc="
                                + lexical_cast<string>(rc));
        }
    }

    void TearDown() {
        pthread_mutex_destroy(m);
        delete m;
    }

    pthread_mutex_t* m;
};

TEST_F(lock_guard_fixture, testLockGuard)
{
    LockGuard<pthread_mutex_t> lg(m);
}
