#include "logger.hpp"
#include <chrono>
#include <iomanip>

Logger::Logger(std::ostream &out) : out_(out) {}

void Logger::log(LogLevel level, const std::string &msg) {
    out_ << timestamp() << " " << level_to_string(level) << " " << msg << '\n';
}

void Logger::debug(const std::string &msg) { log(LogLevel::Debug, msg); }
void Logger::info(const std::string &msg) { log(LogLevel::Info, msg); }
void Logger::warn(const std::string &msg) { log(LogLevel::Warn, msg); }
void Logger::error(const std::string &msg) { log(LogLevel::Error, msg); }
std::string Logger::timestamp() {
    using clock = std::chrono::system_clock;
    auto now = clock::now();
    auto t = clock::to_time_t(now);

    std::tm tm{};
    localtime_r(&t, &tm);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    return oss.str();
}
const char *Logger::level_to_string(LogLevel lvl) {
    switch (lvl) {
    case LogLevel::Debug:
        return "[DEBUG]";
    case LogLevel::Info:
        return "[INFO ]";
    case LogLevel::Warn:
        return "[WARN ]";
    case LogLevel::Error:
        return "[ERROR]";
    }
    return "[???? ]";
}