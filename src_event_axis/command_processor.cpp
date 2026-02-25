#include "command_processor.hpp"
#include "app_context.hpp"

#include <cstdio>

CommandProcessor::CommandProcessor() {
    _handlers["GE"] = handleGE;
    _handlers["GE2"] = handleGE2;
    _handlers["GS"] = handleGS;
    _handlers["GK"] = handleGK;
    _handlers["GL"] = handleGL;
}

std::string CommandProcessor::execute(const std::string& cmd, AppContext* ctx) {
    if (_handlers.count(cmd)) {
        return _handlers[cmd](ctx);
    }
    return "{\"error\": \"unknown_command\"}\n";
}

std::string CommandProcessor::handleGE(AppContext* ctx) {
    pthread_mutex_lock(&ctx->results.lock);
    int count = ctx->results.objects_detected;
    double score = ctx->results.last_score;
    pthread_mutex_unlock(&ctx->results.lock);

    char resp[128];
    sprintf(resp, "{\"ge\": %d, \"score\": %.2f}\n", count, score);
    return std::string(resp);
}

std::string CommandProcessor::handleGE2(AppContext* ctx) {
    pthread_mutex_lock(&ctx->results.lock);
    int count = ctx->results.objects_detected;
    double score = ctx->results.last_score;
    pthread_mutex_unlock(&ctx->results.lock);

    char resp[128];
    sprintf(resp, "{\"ge2\": %d, \"score\": %.2f}\n", count, score);
    return std::string(resp);
}

std::string CommandProcessor::handleGS(AppContext* ctx) {
    pthread_mutex_lock(&ctx->results.lock);
    int count = ctx->results.objects_detected;
    double score = ctx->results.last_score;
    pthread_mutex_unlock(&ctx->results.lock);

    char resp[128];
    sprintf(resp, "{\"gs\": %d, \"score\": %.2f}\n", count, score);
    return std::string(resp);
}

std::string CommandProcessor::handleGK(AppContext* ctx) {
    pthread_mutex_lock(&ctx->results.lock);
    int count = ctx->results.objects_detected;
    double score = ctx->results.last_score;
    pthread_mutex_unlock(&ctx->results.lock);

    char resp[128];
    sprintf(resp, "{\"gk\": %d, \"score\": %.2f}\n", count, score);
    return std::string(resp);
}

std::string CommandProcessor::handleGL(AppContext* ctx) {
    pthread_mutex_lock(&ctx->results.lock);
    int count = ctx->results.objects_detected;
    double score = ctx->results.last_score;
    pthread_mutex_unlock(&ctx->results.lock);

    char resp[128];
    sprintf(resp, "{\"gl\": %d, \"score\": %.2f}\n", count, score);
    return std::string(resp);
}
