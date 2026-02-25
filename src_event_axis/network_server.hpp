#pragma once

#include "app_context.hpp"

class NetworkServer {
public:
    static bool start(AppContext* ctx, int http_port, int tcp_port);
};
