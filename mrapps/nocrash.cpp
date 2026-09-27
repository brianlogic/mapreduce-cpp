//
// same as crash.cpp but doesn't actually crash.
//
// built by the Makefile into nocrash.so
//

#include <algorithm>
#include <string>
#include <vector>

#include "mr/mr.h"

static std::vector<mr::KeyValue> Map(const std::string& filename, const std::string& contents) {
    return {
        {"a", filename},
        {"b", std::to_string(filename.size())},
        {"c", std::to_string(contents.size())},
        {"d", "xyzzy"},
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
