#pragma once

#include "app_context.hpp"
// #include "capturehandler.hpp"
// #include "traffcounter.hpp"

class MathEngine {
public:
    static void* run_thread(void* arg);

private:
    static void processing_loop(AppContext* ctx);
};
