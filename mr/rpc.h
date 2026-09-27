#pragma once
//
// RPC definitions.
//
// these structs travel as raw bytes, so keep them trivially copyable:
// ints, enums, bools and fixed-size char arrays (e.g. char filename[256]).
// no std::string / std::vector.
//

namespace mr {

//
// example to show how to declare the arguments
// and reply for an RPC.
//

struct ExampleArgs {
    int x;
};

struct ExampleReply {
    int y;
};

// Add your RPC definitions here.

} // namespace mr
