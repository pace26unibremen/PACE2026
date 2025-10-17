#include "Logger.hpp"
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/filter_parser.hpp>
#include <fstream>
#include <streambuf>

namespace logging = boost::log;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

using logging::trivial::severity_level;

BOOST_LOG_ATTRIBUTE_KEYWORD(channel, "Channel", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "Timestamp", boost::posix_time::ptime)

std::string logFilterPath = LOG_FILTER_PATH;
std::string logFilePath = LOG_FILE_PATH;

/**
 * A customized formatter for th log records
 * @param rec a log record
 * @param strm in this stream the log should be written
 */
void custom_formatter(logging::record_view const & rec,
                      logging::formatting_ostream & strm)
{
      auto format_time = [&rec, &strm]()
      {
            auto time =
                logging::extract<boost::posix_time::ptime>("Timestamp", rec);
            std::locale loc(std::locale::classic(),
                            new boost::posix_time::time_facet("%H:%M:%S"));
            strm.imbue(loc);
            strm << "[" << time << "]";
      };

      auto format_channel = [&rec, &strm]()
      {
            auto chl = logging::extract<std::string>("Channel", rec);
            if (chl != "Default")
                  strm << " [" << chl << "]";
      };

      auto format_severity = [&rec, &strm]()
      {
            auto sev = logging::extract<severity_level>("Severity", rec);
            if (sev != severity_level::trace)
                  strm << " [" << sev << "]";
      };

      auto format_message = [&rec, &strm]()
      {
            auto sev = logging::extract<severity_level>("Severity", rec);
            if (sev == severity_level::trace || sev == severity_level::info)
            {
                  strm << " " << rec[expr::smessage];
            }
            else
            {
                  auto indentation = std::string(4, ' ');
                  auto msg = "\n" + std::string(*rec[expr::smessage]);
                  boost::algorithm::replace_all(msg, "\n", "\n" + indentation);
                  strm << msg;
            }
      };

      format_time();
      format_channel();
      format_severity();
      format_message();
}

/**
 * init the global logger "logger"
 * @return the logger
 */
BOOST_LOG_GLOBAL_LOGGER_INIT(Logger, logger_t)
{
      std::ifstream file(logFilterPath);
      auto custom_filter = logging::parse_filter(
          std::string(std::istreambuf_iterator<char>(file),
                      std::istreambuf_iterator<char>()));

      logger_t lg;
      lg.add_attribute("Timestamp", attrs::local_clock {});
      logging::add_console_log(std::clog, keywords::format = custom_formatter,
                               keywords::filter = custom_filter);
      logging::add_file_log(keywords::file_name = logFilePath,
                            keywords::format = custom_formatter,
                            keywords::filter = custom_filter);
      return lg;
}