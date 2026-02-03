#ifndef LOG_H
#define LOG_H

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <memory>

namespace SerGame {

    class Log {
    public:
        static void Init();

        inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
        inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
    };

}

#define SER_CORE_TRACE(...)    ::SerGame::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define SER_CORE_INFO(...)     ::SerGame::Log::GetCoreLogger()->info(__VA_ARGS__)
#define SER_CORE_WARN(...)     ::SerGame::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define SER_CORE_ERROR(...)    ::SerGame::Log::GetCoreLogger()->error(__VA_ARGS__)
#define SER_CORE_FATAL(...)    ::SerGame::Log::GetCoreLogger()->critical(__VA_ARGS__)

#define SER_TRACE(...)         ::SerGame::Log::GetClientLogger()->trace(__VA_ARGS__)
#define SER_INFO(...)          ::SerGame::Log::GetClientLogger()->info(__VA_ARGS__)
#define SER_WARN(...)          ::SerGame::Log::GetClientLogger()->warn(__VA_ARGS__)
#define SER_ERROR(...)         ::SerGame::Log::GetClientLogger()->error(__VA_ARGS__)
#define SER_FATAL(...)         ::SerGame::Log::GetClientLogger()->critical(__VA_ARGS__)

#endif
