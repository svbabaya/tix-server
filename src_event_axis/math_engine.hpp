#pragma once

#include "app_context.hpp"

class MathEngine {
public:
    static void* run_thread(void* arg);

private:
    static void processing_loop(AppContext* ctx);
};
