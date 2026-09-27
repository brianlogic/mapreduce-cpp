//
// a MapReduce pseudo-application to test that workers
// execute map tasks in parallel.
//
// built by the Makefile into mtiming.so
//

#include <dirent.h>
#include <signal.h>
#include <unistd.h>

#include <algorithm>
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
    double ts = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
    int pid = getpid();

    int n = nparallel("map");

    char tsbuf[64];
    std::snprintf(tsbuf, sizeof tsbuf, "%.1f", ts);
    return {
        {"times-" + std::to_string(pid), tsbuf},
        {"parallel-" + std::to_string(pid), std::to_string(n)},
    };
}

static std::string Reduce(const std::string& key, const std::vector<std::string>& values) {
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
