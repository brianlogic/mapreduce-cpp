#pragma once
//
// load the application Map and Reduce functions
// from a plugin file, e.g. ../mrapps/wc.so
// (the C++ stand-in for Go's plugin package).
//
// Please do not change this file.
//

#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "mr/mr.h"

namespace mr {

inline Plugin loadPlugin(std::string filename) {
    // dlopen only searches the current directory if the path has a slash.
    if (filename.find('/') == std::string::npos) {
        filename = "./" + filename;
    }
    void* h = dlopen(filename.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (h == nullptr) {
        std::fprintf(stderr, "cannot load plugin %s: %s\n", filename.c_str(), dlerror());
        std::exit(1);
    }
    auto* p = static_cast<const Plugin*>(dlsym(h, "mr_plugin"));
    if (p == nullptr) {
        std::fprintf(stderr, "cannot find mr_plugin in %s\n", filename.c_str());
        std::exit(1);
    }
    return *p;
}

} // namespace mr
