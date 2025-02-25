#pragma once

#include <source_location>
#include <string_view>

namespace mute::log
{
    void debug(std::string_view message, std::source_location location = std::source_location::current());
    void info(std::string_view message, std::source_location location = std::source_location::current());
    void warn(std::string_view message, std::source_location location = std::source_location::current());
    void error(std::string_view message, std::source_location location = std::source_location::current());
}