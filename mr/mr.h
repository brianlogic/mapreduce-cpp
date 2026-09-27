#pragma once
//
// Shared MapReduce types. Please do not change this file.
//

#include <cstdint>
#include <string>
#include <vector>

namespace mr {

//
// Map functions return a vector of KeyValue.
//
struct KeyValue {
    std::string key;
    std::string value;
};

using MapFunc = std::vector<KeyValue> (*)(const std::string& filename, const std::string& contents);
using ReduceFunc = std::string (*)(const std::string& key, const std::vector<std::string>& values);

//
// every mrapps/*.so exports one of these under the C name "mr_plugin".
//
struct Plugin {
    MapFunc map;
    ReduceFunc reduce;
};

//
// use ihash(key) % nReduce to choose the reduce
// task number for each KeyValue emitted by Map.
// (FNV-1a, same as the Go lab.)
//
inline int ihash(const std::string& key) {
    uint32_t h = 2166136261u;
    for (unsigned char c : key) {
        h ^= c;
        h *= 16777619u;
    }
    return static_cast<int>(h & 0x7fffffff);
}

//
// implemented in mr/worker.cpp; main/mrworker.cpp calls it.
//
void Worker(MapFunc mapf, ReduceFunc reducef);

} // namespace mr
