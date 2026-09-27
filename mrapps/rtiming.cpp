//
// a MapReduce pseudo-application to test that workers
// execute reduce tasks in parallel.
//
// built by the Makefile into rtiming.so
//

#include <dirent.h>
#include <signal.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>
#include <vector>

#include "mr/mr.h"

static int nparallel(const std::string& phase) {
    // create a file so that other workers will see that
    // we're running at the same time as them.
    std::string prefix = "mr-worker-" + phase + "-";
    std::string myfilename = prefix + std::to_string(getpid());
    std::FILE* f = std::fopen(myfilename.c_str(), "w");
    if (f == nullptr) {
        std::perror(myfilename.c_str());
        std::abort();
    }
    std::fputs("x", f);
    std::fclose(f);

    // are any other workers running?
    // find their PIDs by scanning directory for mr-worker-XXX files.
    DIR* dd = opendir(".");
    if (dd == nullptr) {
        std::perror("opendir");
        std::abort();
    }
    int ret = 0;
    while (dirent* e = readdir(dd)) {
        std::string name = e->d_name;
        if (name.compare(0, prefix.size(), prefix) != 0) {
            continue;
        }
        int xpid = std::atoi(name.c_str() + prefix.size());
        // kill(pid, 0) succeeds if xpid is alive.
        if (xpid > 0 && kill(xpid, 0) == 0) {
            ret += 1;
        }
    }
    closedir(dd);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (std::remove(myfilename.c_str()) != 0) {
        std::perror(myfilename.c_str());
        std::abort();
    }

    return ret;
}

static std::vector<mr::KeyValue> Map(const std::string& filename, const std::string& contents) {
    std::vector<mr::KeyValue> kva;
    for (const char* k : {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j"}) {
        kva.push_back({k, "1"});
    }
    return kva;
}

static std::string Reduce(const std::string& key, const std::vector<std::string>& values) {
    int n = nparallel("reduce");

    return std::to_string(n);
}

extern "C" const mr::Plugin mr_plugin = {Map, Reduce};
