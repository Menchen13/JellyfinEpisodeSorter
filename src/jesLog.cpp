#include <iostream>
#include <memory>

#include <spdlog/async.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <jesLog.hpp>

namespace fs = std::filesystem;

namespace JES {
constexpr spdlog::level::level_enum get_compiled_log_level() {
#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_TRACE
  return spdlog::level::trace;
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_DEBUG
  return spdlog::level::debug;
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_INFO
  return spdlog::level::info;
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_WARN
  return spdlog::level::warn;
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_ERROR
  return spdlog::level::err;
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_CRITICAL
  return spdlog::level::critical;
#else
  return spdlog::level::off;
#endif
}

fs::path resolveLogPath() {
  fs::path logDir;
#ifndef NDEBUG
  // Release Build Path
#ifdef _WIN32
  if (const char *localAppData = std::getenv("LOCALAPPDATA")) {
    logDir = fs::path(localAppData) / "JES" / "logs";
  } else {
    logDir = fs::current_path() / "logs";
  }
#else
  if (const char *xdgState = std::getenv("XDG_STATE_HOME")) {
    logDir = fs::path(xdgState) / "JES" / "logs";
  } else if (const char *home = std::getenv("HOME")) {
    logDir = fs::path(home) / ".local" / "state" / "JES";
  } else {
    logDir = fs::current_path() / "logs";
  }
#endif

#else
  // Debug Build Path
  logDir = fs::current_path() / "logs";

#endif // !NDEBUG

  fs::create_directories(logDir);
  return logDir / "JES.log";
}

std::shared_ptr<spdlog::logger> getLibLogger() {
  static std::shared_ptr<spdlog::logger> logger =
      []() -> std::shared_ptr<spdlog::logger> {
    try {
      // if lib logger was already createy by app using lib for some reason
      auto existing = spdlog::get("JESLibLogger");
      if (existing)
        return existing;

      // create console Sink level warn or higher by default
      auto consoleSink =
          std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
#ifndef NDEBUG
      consoleSink->set_level(spdlog::level::warn);
#else
      consoleSink->set_level(spdlog::level::info);
#endif // !NDEBUG
      consoleSink->set_pattern("[%^JES%$] [%H:%M:%S] [%l] %v");

      auto fileSink =
          std::make_shared<spdlog::sinks::basic_file_sink_mt>(resolveLogPath());
      fileSink->set_level(get_compiled_log_level());

#ifndef NDEBUG
      fileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%l] %v");
#else
      fileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%l] [%@] [%!] %v");
#endif // !NDEBUG

      std::vector<spdlog::sink_ptr> sinks{consoleSink, fileSink};
      auto newLogger = std::make_shared<spdlog::logger>(
          "JESLibLogger", sinks.begin(), sinks.end());

      newLogger->set_level(get_compiled_log_level());

      spdlog::register_logger(newLogger);

      return newLogger;

    } catch (const spdlog::spdlog_ex &ex) {
      // SOMETHING WENT WRONG (e.g., "logs/" folder doesn't exist or is
      // read-only)

      // Print to standard error so the dev can see it in the terminal
      std::cerr << "[MY_LIB CRITICAL] Failed to initialize logging: "
                << ex.what() << "\n";
      std::cerr << "[MY_LIB CRITICAL] Falling back to a Null Logger. Logs will "
                   "be lost.\n";

      // Return a Null Logger.
      auto null_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
      return std::make_shared<spdlog::logger>("my_cool_lib_null", null_sink);
    }
  }();
  return logger;
}
void initLog(){getLibLogger();}
} // namespace JES
