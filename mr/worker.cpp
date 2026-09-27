#include <cstdio>

#include "mr/mr.h"
#include "mr/rpc.h"
#include "mr/rpc_lib.h"

namespace mr {

//
// example function to show how to make an RPC call to the coordinator.
//
// the RPC argument and reply types are defined in rpc.h.
//
void CallExample() {
    // declare an argument structure.
    ExampleArgs args{};

    // fill in the argument(s).
    args.x = 99;

    // declare a reply structure.
    ExampleReply reply{};

    // send the RPC request, wait for the reply.
    // "Coordinator.Example" is the name the coordinator
    // registered the handler under in Coordinator::server().
    bool ok = call("Coordinator.Example", args, reply);
    if (ok) {
        // reply.y should be 100.
        std::printf("reply.y %d\n", reply.y);
    } else {
        std::printf("call failed!\n");
    }
}

//
// main/mrworker.cpp calls this function.
//
void Worker(MapFunc mapf, ReduceFunc reducef) {
    // Your worker implementation here.

    // uncomment to send the Example RPC to the coordinator.
    // CallExample();
}

} // namespace mr
