#pragma once

#ifndef PACE2025_LOGGER_HPP
#define PACE2025_LOGGER_HPP

#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/attributes/attribute.hpp>

typedef boost::log::sources::severity_channel_logger_mt<boost::log::trivial::severity_level> logger_t;

/**
 * The path to the file into which the log records are written.\n
 * Change it before first LOG record.\n
 * Default: "{ProjectRoot}/.log"
 */
extern std::string logFilePath;

/**
 * The path to the file into which the filter for the log record is defined.\n
 * Change it before first LOG record.\n
 * Default: "{ProjectRoot}/log_filter.ini"
 */
extern std::string logFilterPath;

// Declares the global logger "logger" from type logger_t
BOOST_LOG_GLOBAL_LOGGER(Logger, logger_t)

/// \def
/// "LOG_CH(channel) \<\< msg"  creates a log record for the specific channel
#define LOG_CH(channel) BOOST_LOG_CHANNEL(Logger::get(), channel)

/// \def
/// "LOG_ERR_CH(channel) \<\< msg"  creates an error log record for the specific channel
#define LOG_ERR_CH(channel)   BOOST_LOG_CHANNEL_SEV(Logger::get(), channel,  boost::log::trivial::severity_level::error)

/// \def
/// "LOG_WRN_CH(channel) \<\< msg"  creates a warning log record for the specific channel
#define LOG_WRN_CH(channel) BOOST_LOG_CHANNEL_SEV(Logger::get(), channel,  boost::log::trivial::severity_level::warning)

/// \def
/// "LOG_INF_CH(channel) \<\< msg"  creates an info log record for the specific channel
#define LOG_INF_CH(channel)    BOOST_LOG_CHANNEL_SEV(Logger::get(), channel,  boost::log::trivial::severity_level::info)

/// \def
/// "LOG \<\< msg"  creates a log record, channel name will be "Default"
#define LOG BOOST_LOG_CHANNEL_SEV(Logger::get(), "Default", boost::log::trivial::severity_level::trace)

/// \def
/// "LOG_ERR \<\< msg"  creates an error log record, channel name will be "Default"
#define LOG_ERR BOOST_LOG_CHANNEL_SEV(Logger::get(), "Default",  boost::log::trivial::severity_level::error)

/// \def
/// "LOG_WRN \<\< msg"  creates a warning log record, channel name will be "Default"
#define LOG_WRN BOOST_LOG_CHANNEL_SEV(Logger::get(), "Default",  boost::log::trivial::severity_level::warning)

/// \def
/// "LOG_INF \<\< msg"  creates an info record, channel name will be "Default"
#define LOG_INF BOOST_LOG_CHANNEL_SEV(Logger::get(), "Default",  boost::log::trivial::severity_level::info)

#endif   // PACE2025_LOGGER_HPP