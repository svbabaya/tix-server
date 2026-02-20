#ifndef NETWORK_SERVER_HPP
#define NETWORK_SERVER_HPP

#include "app300_context.hpp"

class NetworkServer {
public:
    static bool start(AppContext* ctx, int http_port, int tcp_port);
};

#endif
