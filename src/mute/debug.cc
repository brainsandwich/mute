#include "mute/debug.h"

#include <fmt/printf.h>

namespace mute::log
{
    enum class LogLevel {
        Debug,
        Info,
        Warn,
        Error
    };

    std::string locationString(const std::source_location& location)
    {
        return fmt::format("{}:{}", location.file_name(), location.line());
    }

    void logmsg(LogLevel level, std::string_view message, const std::source_location& location)
    {
#if !defined(DEBUG)
        switch (level)
        {
            case LogLevel::Debug: fmt::println("DEBUG [{}]: {}", locationString(location), message); break;
            case LogLevel::Info: fmt::println("INFO [{}]: {}", locationString(location), message); break;
            case LogLevel::Warn: fmt::println("WARN [{}]: {}", locationString(location), message); break;
            case LogLevel::Error: fmt::println("ERROR [{}]: {}", locationString(location), message); break;
        }
#else
    switch (level)
        {
            case LogLevel::Debug: fmt::println("DEBUG: {}", message); break;
            case LogLevel::Info: fmt::println("INFO: {}", message); break;
            case LogLevel::Warn: fmt::println("WARN: {}", message); break;
            case LogLevel::Error: fmt::println("ERROR: {}", message); break;
        }
#endif
    }

    void debug(std::string_view message, std::source_location location) { logmsg(LogLevel::Debug, message, location); }
    void info(std::string_view message, std::source_location location) { logmsg(LogLevel::Info, message, location); }
    void warn(std::string_view message, std::source_location location) { logmsg(LogLevel::Warn, message, location); }
    void error(std::string_view message, std::source_location location) { logmsg(LogLevel::Error, message, location); }
}