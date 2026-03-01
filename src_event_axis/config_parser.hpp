#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include "algo_params.hpp"

// Объявление функции, чтобы другие файлы её видели
GlobalConfig parseJsonToConfig(const char* jsonString);

#endif // CONFIG_PARSER_HPP
