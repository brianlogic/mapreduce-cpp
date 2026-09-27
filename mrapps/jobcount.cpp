//
// a MapReduce pseudo-application that counts the number of times map/reduce
// tasks are run, to test whether jobs are assigned multiple times even when
// there is no failure.
//
// built by the Makefile into jobcount.so
//

#include <dirent.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "mr/mr.h"

static int count;

static std::vector<mr::KeyValue> Map(const std::string& filename, const std::string& contents) {
    std::string f = "mr-worker-jobcount-" + std::to_string(getpid()) + "-" + std::to_string(count);
    count++;
    std::FILE* fp = std::fopen(f.c_str(), "w");
    if (fp == nullptr) {
        std::perror(f.c_str());
        std::abort();
    }
    std::fputs("x", fp);
    std::fclose(fp);

    std::random_device rd;
    int ms = 2000 + std::uniform_int_distribution<int>(0, 2999)(rd);
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    return {{"a", "x"}};
}

static std::string Reduce(const std::string& key, const std::vector<std::string>& values) {
    DIR* dd = opendir(".");
    if (dd == nullptr) {
        std::perror("opendir");
        std::abort();
    }
    int invocations = 0;
    while (dirent* e = readdir(dd)) {
        if (std::string(e->d_name).rfind("mr-worker-jobcount", 0) == 0) {
            invocations++;
        }
    }
    closedir(dd);
    return std::to_string(invocations);
}

extern "C" const mr::Plugin mr_plugin = {Map, Reduce};
