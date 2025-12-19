#ifndef ELEVEN_STICKS_LOGGER_HPP
#define ELEVEN_STICKS_LOGGER_HPP

#include <iostream>
#include <sstream>
#include <string>

enum class LogLevel { Debug, Info, Warn, Error };

class Logger {
  public:
    explicit Logger(std::ostream &out = std::clog);

    void log(LogLevel level, const std::string &msg);

    void debug(const std::string &msg);
    void info(const std::string &msg);
    void warn(const std::string &msg);
    void error(const std::string &msg);

  private:
    std::ostream &out_;
    static std::string timestamp();

    static const char *level_to_string(LogLevel lvl);
};

#endif