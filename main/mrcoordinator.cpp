//
// start the coordinator process, which is implemented
// in ../mr/coordinator.cpp
//
// ./mrcoordinator pg*.txt
//
// Please do not change this file.
//

#include <chrono>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

#include "mr/coordinator.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "Usage: mrcoordinator inputfiles...\n");
        return 1;
    }

    std::vector<std::string> files(argv + 1, argv + argc);

    // never deleted: RPC handler threads may still be using it when main returns.
    auto* m = new mr::Coordinator(files, 10);
    while (!m->Done()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
