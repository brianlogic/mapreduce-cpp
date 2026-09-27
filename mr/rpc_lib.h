#pragma once
//
// A tiny RPC library over a UNIX-domain socket -- the C++ stand-in
// for Go's net/rpc. Please do not change this file.
//
// Server side (coordinator):
//     rpc_.handle("Coordinator.Example", this, &Coordinator::Example);
//     rpc_.serve();
//
// Client side (worker):
//     ExampleArgs args{};
//     ExampleReply reply{};
//     bool ok = call("Coordinator.Example", args, reply);
//
// Args and Reply structs are sent as raw bytes, so they must be
// trivially copyable: ints, enums, bools, fixed-size char arrays.
// No std::string or std::vector inside them.
//

#include <cstring>
#include <functional>
#include <map>
#include <string>
#include <type_traits>

namespace mr {

// Cook up a unique-ish UNIX-domain socket name
// in /var/tmp, for the coordinator.
std::string coordinatorSock();

// untyped transport; use call() below instead.
bool rpcCallRaw(const std::string& method, const std::string& args, std::string& reply);

class RpcServer {
  public:
    // register obj->fn to serve `method`. fn must look like
    //     void Coordinator::Foo(const FooArgs& args, FooReply& reply);
    // register every handler before calling serve().
    template <class C, class Args, class Reply>
    void handle(const std::string& method, C* obj, void (C::*fn)(const Args&, Reply&)) {
        static_assert(std::is_trivially_copyable_v<Args> && std::is_trivially_copyable_v<Reply>,
                      "RPC structs must be trivially copyable (no std::string/std::vector)");
        handlers_[method] = [obj, fn](const std::string& in, std::string& out) {
            if (in.size() != sizeof(Args)) {
                return false;
            }
            Args args;
            std::memcpy(&args, in.data(), sizeof args);
            Reply reply{};
            (obj->*fn)(args, reply);
            out.assign(reinterpret_cast<const char*>(&reply), sizeof reply);
            return true;
        };
    }

    // start a thread that listens on coordinatorSock().
    // like Go, every incoming RPC runs on its own thread,
    // so handlers run concurrently -- lock your shared state.
    void serve();

  private:
    void serveConn(int fd);
    std::map<std::string, std::function<bool(const std::string&, std::string&)>> handlers_;
};

//
// send an RPC request to the coordinator, wait for the response.
// returns false if the coordinator can't be reached (e.g. it has
// exited because the job is done) or the call fails.
//
template <class Args, class Reply>
bool call(const std::string& method, const Args& args, Reply& reply) {
    static_assert(std::is_trivially_copyable_v<Args> && std::is_trivially_copyable_v<Reply>,
                  "RPC structs must be trivially copyable (no std::string/std::vector)");
    std::string in(reinterpret_cast<const char*>(&args), sizeof args);
    std::string out;
    if (!rpcCallRaw(method, in, out) || out.size() != sizeof reply) {
        return false;
    }
    std::memcpy(&reply, out.data(), sizeof reply);
    return true;
}

} // namespace mr
