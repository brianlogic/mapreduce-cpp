//
// RPC transport. Please do not change this file.
//
// wire format, one call per connection:
//   client -> server: [u32 len][method] [u32 len][args bytes]
//   server -> client: [u32 len][reply bytes]
// on any error the server just closes the connection.
//

#include "mr/rpc_lib.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <thread>

namespace mr {

std::string coordinatorSock() {
    return "/var/tmp/5840-mr-" + std::to_string(getuid());
}

static sockaddr_un sockAddr() {
    sockaddr_un a{};
    a.sun_family = AF_UNIX;
    std::string path = coordinatorSock();
    std::strncpy(a.sun_path, path.c_str(), sizeof a.sun_path - 1);
    return a;
}

static bool writeAll(int fd, const void* buf, size_t n) {
    auto* p = static_cast<const char*>(buf);
    while (n > 0) {
        ssize_t w = ::write(fd, p, n);
        if (w < 0 && errno == EINTR) {
            continue;
        }
        if (w <= 0) {
            return false;
        }
        p += w;
        n -= w;
    }
    return true;
}

static bool readAll(int fd, void* buf, size_t n) {
    auto* p = static_cast<char*>(buf);
    while (n > 0) {
        ssize_t r = ::read(fd, p, n);
        if (r < 0 && errno == EINTR) {
            continue;
        }
        if (r <= 0) {
            return false;
        }
        p += r;
        n -= r;
    }
    return true;
}

static bool writeMsg(int fd, const std::string& s) {
    uint32_t n = s.size();
    return writeAll(fd, &n, sizeof n) && writeAll(fd, s.data(), n);
}

static bool readMsg(int fd, std::string& s) {
    uint32_t n;
    if (!readAll(fd, &n, sizeof n) || n > (1u << 20)) {
        return false;
    }
    s.resize(n);
    return n == 0 || readAll(fd, &s[0], n);
}

bool rpcCallRaw(const std::string& method, const std::string& args, std::string& reply) {
    // writing to a socket the coordinator closed would otherwise kill us.
    std::signal(SIGPIPE, SIG_IGN);
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        return false;
    }
    sockaddr_un a = sockAddr();
    bool ok = connect(fd, reinterpret_cast<sockaddr*>(&a), sizeof a) == 0 &&
              writeMsg(fd, method) && writeMsg(fd, args) && readMsg(fd, reply);
    close(fd);
    return ok;
}

void RpcServer::serve() {
    std::signal(SIGPIPE, SIG_IGN);
    std::string path = coordinatorSock();
    unlink(path.c_str());
    int lfd = socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un a = sockAddr();
    if (lfd < 0 || bind(lfd, reinterpret_cast<sockaddr*>(&a), sizeof a) < 0 || listen(lfd, 128) < 0) {
        std::perror("rpc listen");
        std::exit(1);
    }
    std::thread([this, lfd] {
        for (;;) {
            int fd = accept(lfd, nullptr, nullptr);
            if (fd < 0) {
                if (errno != EINTR) {
                    std::perror("rpc accept");
                }
                continue;
            }
            std::thread([this, fd] {
                serveConn(fd);
                close(fd);
            }).detach();
        }
    }).detach();
}

void RpcServer::serveConn(int fd) {
    std::string method, args, reply;
    if (!readMsg(fd, method) || !readMsg(fd, args)) {
        return;
    }
    auto it = handlers_.find(method);
    if (it == handlers_.end()) {
        std::fprintf(stderr, "rpc: unknown method %s\n", method.c_str());
        return;
    }
    if (!it->second(args, reply)) {
        std::fprintf(stderr, "rpc: %s called with wrong args type\n", method.c_str());
        return;
    }
    writeMsg(fd, reply);
}

} // namespace mr
