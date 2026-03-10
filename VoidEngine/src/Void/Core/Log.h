#ifndef LOG_H
#define LOG_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

namespace Void {

    class Log {
    public:
        static void Init() {
            spdlog::set_pattern("%^[%T] %n: %v%$");
            s_CoreLogger = spdlog::stdout_color_mt("VOID");
            s_CoreLogger->set_level(spdlog::level::trace);
        }

        inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_CoreLogger; }

    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
    };

}

#define VOID_CORE_TRACE(...) ::Void::Log::GetLogger()->trace(__VA_ARGS__)
#define VOID_CORE_INFO(...)  ::Void::Log::GetLogger()->info(__VA_ARGS__)
#define VOID_CORE_WARN(...)  ::Void::Log::GetLogger()->warn(__VA_ARGS__)
#define VOID_CORE_ERROR(...) ::Void::Log::GetLogger()->error(__VA_ARGS__)
#define VOID_CORE_FATAL(...) ::Void::Log::GetLogger()->critical(__VA_ARGS__)

#define VOID_TRACE(...) ::Void::Log::GetLogger()->trace(__VA_ARGS__)
#define VOID_INFO(...)  ::Void::Log::GetLogger()->info(__VA_ARGS__)
#define VOID_WARN(...)  ::Void::Log::GetLogger()->warn(__VA_ARGS__)
#define VOID_ERROR(...) ::Void::Log::GetLogger()->error(__VA_ARGS__)
#define VOID_FATAL(...) ::Void::Log::GetLogger()->critical(__VA_ARGS__)

#endif
