#pragma once

#include <string>
#include <map>

// 1. Опережающее объявление (вместо include)
class AppContext; 

// 2. Исправленный typedef (убираем лишнее присваивание)
typedef std::string (*HandlerFunc)(AppContext* ctx);

class CommandProcessor {
public:
    CommandProcessor();
    
    // 3. Используем указатель на AppContext
    std::string execute(const std::string& cmd, AppContext* ctx);

private:
    // Теперь это скомпилируется корректно
    std::map<std::string, HandlerFunc> _handlers;
    
    static std::string handleGE(AppContext* ctx);
    static std::string handleGE2(AppContext* ctx);
    static std::string handleGS(AppContext* ctx);
    static std::string handleGK(AppContext* ctx);
    static std::string handleGL(AppContext* ctx);
};
