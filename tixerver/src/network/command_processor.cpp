#include "command_processor.hpp"
#include "app_context.hpp"

#include <mutex>
#include <cstdio>

CommandProcessor::CommandProcessor() {
    _handlers["GE"] = handleGE;
    _handlers["GE2"] = handleGE2;
    _handlers["GS"] = handleGS;
    _handlers["GK"] = handleGK;
    _handlers["GL"] = handleGL;
}

std::string CommandProcessor::execute(const std::string& cmd, AppContext* ctx) {
    auto it = _handlers.find(cmd);
    if (it != _handlers.end()) {
        return it->second(ctx);
    }
    return "{\"error\": \"unknown_command\"}\n";
}

// Вспомогательная функция, чтобы не дублировать sprintf (опционально)
static std::string formatResponse(const char* label, int count, double score) {
    char resp[128];
    // snprintf безопаснее старого sprintf
    snprintf(resp, sizeof(resp), "{\"%s\": %d, \"score\": %.2f}\n", label, count, score);
    return std::string(resp);
}

std::string CommandProcessor::handleGE(AppContext* ctx) {
    int count;
    double score;
    {
        // RAII-блокировка: захват при создании, освобождение при выходе из {}
        std::lock_guard<std::mutex> lock(ctx->results.lock);
        count = ctx->results.objects_detected;
        score = ctx->results.last_score;
    } 
    return formatResponse("ge", count, score);
}

std::string CommandProcessor::handleGE2(AppContext* ctx) {
    int count;
    double score;
    {
        std::lock_guard<std::mutex> lock(ctx->results.lock);
        count = ctx->results.objects_detected;
        score = ctx->results.last_score;
    }
    return formatResponse("ge2", count, score);
}

std::string CommandProcessor::handleGS(AppContext* ctx) {
    int count;
    double score;
    {
        std::lock_guard<std::mutex> lock(ctx->results.lock);
        count = ctx->results.objects_detected;
        score = ctx->results.last_score;
    }
    return formatResponse("gs", count, score);
}

std::string CommandProcessor::handleGK(AppContext* ctx) {
    int count;
    double score;
    {
        std::lock_guard<std::mutex> lock(ctx->results.lock);
        count = ctx->results.objects_detected;
        score = ctx->results.last_score;
    }
    return formatResponse("gk", count, score);
}

std::string CommandProcessor::handleGL(AppContext* ctx) {
    int count;
    double score;
    {
        std::lock_guard<std::mutex> lock(ctx->results.lock);
        count = ctx->results.objects_detected;
        score = ctx->results.last_score;
    }
    return formatResponse("gl", count, score);
}
