#pragma once

#include <string>
#include <vector>

#include "mr/rpc.h"
#include "mr/rpc_lib.h"

namespace mr {

class Coordinator {
  public:
    //
    // create a Coordinator.
    // main/mrcoordinator.cpp calls this.
    // nReduce is the number of reduce tasks to use.
    //
    Coordinator(const std::vector<std::string>& files, int nReduce);

    //
    // main/mrcoordinator.cpp calls Done() periodically to find out
    // if the entire job has finished.
    //
    bool Done();

    // Your RPC handlers here -- workers call these.
    // each must look like: void Name(const NameArgs& args, NameReply& reply);
    void Example(const ExampleArgs& args, ExampleReply& reply);

  private:
    void server();

    RpcServer rpc_;

    // Your definitions here.
};

} // namespace mr
