#ifndef MATH_ENGINE_HPP
#define MATH_ENGINE_HPP

#include "app_context.hpp"

class MathEngine {
public:
    static void* run_thread(void* arg);

private:
    static void processing_loop(AppContext* ctx);
};

#endif // MATH_ENGINE_HPP
