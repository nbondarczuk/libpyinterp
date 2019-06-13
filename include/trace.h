#ifndef __TRACE__
#define __TRACE__

#include <iostream>
#include <string>

#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

#include "lexical_cast.h"

#ifdef __USE_TRACE__
#define FRAME __Frame__ __t__(__FUNCTION__)
#define INFO(__MESSAGE__) __t__.info(__MESSAGE__)
#define __IS_TRACE__ true
#else
#define FRAME
#define INFO(__MESSAGE__) {}
#define __IS_TRACE__ false
#endif


class __Frame__
{
public:
    __Frame__(const char* fn, const std::string& msg = "") : frame(fn) {
        pid = getpid();
        ++call_level;
        if (is_trace) {
            try {
                std::cerr << note("TRACE", "Enter " + msg) << std::endl;
            } catch(...) {}
        }
    }

    ~__Frame__() {
        if (is_trace) {
            try {
                std::cerr << note("TRACE", "Leave") << std::endl;
            } catch(...) {}
        }
        --call_level;
    }

    void info(const std::string& msg) const {
        if (is_trace) {
            try {
                std::cerr << note("TRACE", msg) << std::endl;
            } catch(...) {}
        }
    }

    std::string note(const std::string& level, const std::string& msg) const {
        std::string sep(msg.length() != 0, ' ');
        pthread_t tid = pthread_self();
        std::string rv = "[" + lexical_cast<std::string>(pid) +
            "][" + lexical_cast<std::string>(tid) + "] " +
            level + " " + prefix() + frame + sep + msg;

        return rv;
    }

    static void on() { is_trace = true; }

    static void off() { is_trace = true; }

private:
    static bool is_trace;
    static unsigned int call_level;
    std::string frame;
    pid_t pid;
    static std::string prefix() {std::string s(call_level, ' '); return s;}
};

#endif /* __TRACE__ */
