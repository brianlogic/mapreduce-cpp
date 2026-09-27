//
// start a worker process, which is implemented
// in ../mr/worker.cpp. typically there will be
// multiple worker processes, talking to one coordinator.
//
// ./mrworker ../mrapps/wc.so
//
// Please do not change this file.
//

#include <cstdio>

#include "mr/mr.h"
#include "mr/plugin.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "Usage: mrworker xxx.so\n");
        return 1;
    }

    mr::Plugin p = mr::loadPlugin(argv[1]);

    mr::Worker(p.map, p.reduce);
    return 0;
}
