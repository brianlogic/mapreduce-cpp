#include "mr/coordinator.h"

namespace mr {

// Your code here -- RPC handlers for the worker to call.

//
// an example RPC handler.
//
// the RPC argument and reply types are defined in rpc.h.
//
void Coordinator::Example(const ExampleArgs& args, ExampleReply& reply) {
    reply.y = args.x + 1;
}

//
// start a thread that listens for RPCs from worker.cpp.
// register every handler before rpc_.serve().
//
void Coordinator::server() {
    rpc_.handle("Coordinator.Example", this, &Coordinator::Example);
    rpc_.serve();
}

bool Coordinator::Done() {
    bool ret = false;

    // Your code here.

    return ret;
}

Coordinator::Coordinator(const std::vector<std::string>& files, int nReduce) {
    // Your code here.

    server();
}

} // namespace mr
