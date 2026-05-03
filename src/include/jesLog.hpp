#pragma once

#include <memory>

#include <spdlog/spdlog.h>

// Logging layout
// Debug Build:
// LogLevel for file=TRACE
// LogLevel for terminal=WARN
// LogFile location=[current_path]/logs/JES.log
//
// Release Build:
// LogLevel for file=INFO
// LogLevel for terminal=WARN
// LogFile location=[UserLocalDataFolder]/JES/logs/JES.log

namespace JES {

// creates the the singleton spdlog::logger "JESLibLogger"
std::shared_ptr<spdlog::logger> getLibLogger();

//manually intitalises default spdlog::logger "JESLibLogger"
void initLog();

#define JES_TRACE(...) SPDLOG_LOGGER_TRACE(JES::getLibLogger(), __VA_ARGS__)
#define JES_DEBUG(...) SPDLOG_LOGGER_DEBUG(JES::getLibLogger(), __VA_ARGS__)
#define JES_INFO(...) SPDLOG_LOGGER_INFO(JES::getLibLogger(), __VA_ARGS__)
#define JES_WARN(...) SPDLOG_LOGGER_WARN(JES::getLibLogger(), __VA_ARGS__)
#define JES_ERROR(...) SPDLOG_LOGGER_ERROR(JES::getLibLogger(), __VA_ARGS__)

} // namespace JES
