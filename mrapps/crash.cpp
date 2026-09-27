//
// a MapReduce pseudo-application that sometimes crashes,
// and sometimes takes a long time,
// to test MapReduce's ability to recover.
//
// built by the Makefile into crash.so
//

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "mr/mr.h"

static int randn(int n) {
    std::random_device rd;
    return std::uniform_int_distribution<int>(0, n - 1)(rd);
}

static void maybeCrash() {
    int rr = randn(1000);
    if (rr < 330) {
        // crash! _Exit skips destructors and buffer flushes, like a real crash.
        std::_Exit(1);
    } else if (rr < 660) {
        // delay for a while.
        std::this_thread::sleep_for(std::chrono::milliseconds(randn(10 * 1000)));
    }
}

static std::vector<mr::KeyValue> Map(const std::string& filename, const std::string& contents) {
    maybeCrash();

    return {
        {"a", filename},
        {"b", std::to_string(filename.size())},
        {"c", std::to_string(contents.size())},
        {"d", "xyzzy"},
    };
}

static std::string Reduce(const std::string& key, const std::vector<std::string>& values) {
    maybeCrash();

    // sort values to ensure deterministic output.
    std::vector<std::string> vv = values;
    std::sort(vv.begin(), vv.end());

    std::string val;
    for (size_t i = 0; i < vv.size(); i++) {
        if (i > 0) {
            val += " ";
        }
        val += vv[i];
    }
    return val;
}

extern "C" const mr::Plugin mr_plugin = {Map, Reduce};
