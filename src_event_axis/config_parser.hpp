#pragma once
#include "algo_params.hpp"

// Объявление функции, чтобы другие файлы её видели
GlobalConfig parseJsonToConfig(const char* jsonString);
