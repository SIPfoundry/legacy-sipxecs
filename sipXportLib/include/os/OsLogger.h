#ifndef LOGGER_H
#define	LOGGER_H

#include <sys/time.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <execinfo.h> // for backtrace
#include <dlfcn.h> // for dladdr
#include <cxxabi.h> // for __cxa_demangle

#include <iostream>
#include <sstream>
#include <iosfwd>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/interprocess/detail/atomic.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace boost_filesystem = boost::filesystem;

namespace Os
{

  template <typename T>
  class LogFileChannelBase : boost::noncopyable
  {
  public:
    #define default_mode std::fstream::in | std::fstream::out | std::fstream::binary | std::fstream::app | std::fstream::ate

    LogFileChannelBase() :
      _mode(default_mode)
    {
    }

    LogFileChannelBase(const char* path, std::ios_base::openmode mode = default_mode)
    {
      open(path, mode);
    }

    LogFileChannelBase(const boost_filesystem::path& path, std::ios_base::openmode mode = default_mode)
    {
      open(path.string().c_str(), mode);
    }

    LogFileChannelBase(const std::string& path, std::ios_base::openmode mode = default_mode)
    {
      open(path.c_str(), mode);
    }

    ~LogFileChannelBase()
    {
      _fstream.close();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        // Read up to n characters from the underlying data source
        // into the buffer s, returning the number of characters
        // read; return -1 to indicate EOF
      _fstream.get(s, n);
      return _fstream.gcount();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
     _fstream.write(s, n);
      return n;
    }

    void flush()
    {
      _fstream.flush();
    }

    std::streamsize seek(std::streamsize off, std::ios_base::seekdir way)
    {
      _fstream.seekg(off, way);
      return _fstream.tellg();
    }

    bool open(const char* path, std::ios_base::openmode mode = std::fstream::in | std::fstream::out | std::fstream::app)
    {
      _fstream.close();
      _path = path;
      _mode = mode;
      _fstream.open(_path.string().c_str(), _mode);
      return _fstream.good();
    }

    std::streamsize size()
    {
      // get length of file:
      std::streamsize offset = _fstream.tellg();
      _fstream.seekg(0, std::ios::end);
      std::streamsize size = _fstream.tellg();
      _fstream.seekg(offset);
      return size;
    }

    void open(const boost_filesystem::path& path, std::ios_base::openmode mode = std::fstream::in | std::fstream::out | std::fstream::app)
    {
      open(path.string().c_str(), mode);
    }

    void open(const std::string& path, std::ios_base::openmode mode = std::fstream::in | std::fstream::out | std::fstream::app)
    {
      open(path.c_str(), mode);
    }

    bool is_open()
    {
      return _fstream.is_open();
    }

    void close()
    {
      _fstream.close();
    }

    bool auto_close()
    {
      return false;
    }

    const boost_filesystem::path& path() const
    {
      return _path;
    }

    bool erase()
    {
      _fstream.close();
      try
      {
        boost_filesystem::remove(_path);
      }
      catch(std::exception& e)
      {
        return false;
      }
      return !boost_filesystem::exists(_path);
    }

    bool copy(const boost_filesystem::path& newLocation)
    {
      try
      {
        boost_filesystem::copy_file(_path, newLocation);
      }
      catch(std::exception& e)
      {
        return false;
      }

      return boost_filesystem::exists(newLocation);
    }

    bool move(const boost_filesystem::path& newLocation, bool openNew = false)
    {
      if (!copy(newLocation))
        return false;

      if (erase())
      {
        if (openNew)
          open(newLocation, _mode);
        else
          _path = newLocation;
        return true;
      }
      return false;
    }

    T& stream()
    {
      return _fstream;
    }
  private:
    boost_filesystem::path _path;
    T _fstream;
    std::ios_base::openmode _mode;
  };

  typedef LogFileChannelBase<std::fstream> LogFileChannel;


//
// This filter adds an atomic counter to the log header
//
  class LogCounterFilter
  {
  public:
    LogCounterFilter() :
      _counter(0)
    {
    }

    ~LogCounterFilter()
    {
    }

    bool filter(int facility, int priority, const std::string& task, std::ostringstream& headers, std::string& message)
    {
      headers << boost::interprocess::detail::atomic_inc32(&_counter) + 1 << ":";
      return true;
    }

  public:
    std::string _hostName;
    std::string _processName;

  private:
    volatile boost::uint32_t _counter;

  };

  class LogTimeFilter
  {
  public:
    LogTimeFilter()
    {
    }

    ~LogTimeFilter()
    {
    }

    bool filter(int facility, int priority, const std::string& task, std::ostringstream& headers, std::string& message)
    {
      struct timeval tv;
      struct timezone tz;
      struct tm *tm;
      ::gettimeofday(&tv, &tz);
      tm=::gmtime(&tv.tv_sec);
      char strTime[28];
      ::memset(strTime, '\0', 28);
      ::sprintf(strTime, "%4d-%02d-%02dT%02d:%02d:%02d.%06dZ",
        tm->tm_year + 1900, tm->tm_mon+1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec, (int)tv.tv_usec);

      headers << "\"" << strTime << "\"" << ":";

      return true;
    }
    public:
    std::string _hostName;
    std::string _processName;
  };

  class LogFacilityFilter
  {
  public:
    LogFacilityFilter()
    {
    }

    bool filter(int facility, int priority, const std::string& task, std::ostringstream& headers, std::string& message)
    {
      static const char* facilityList[44] =
      {
         "PERF",
         "KERNEL",
         "AUTH",
         "NET",
         "RTP",
         "PHONESET",
         "HTTP",
         "SIP",
         "CP",
         "MP",
         "TAO",
         "JNI",
         "JAVA",
         "LOG",
         "SUPERVISOR",
         "OUTGOING",
         "INCOMING",
         "INCOMING_PARSED",
         "MEDIASERVER_CGI",
         "MEDIASERVER_VXI",
         "ACD",
         "PARK",
         "APACHE_AUTH",
         "UPGRADE",
         "LINE_MGR",
         "REFRESH_MGR",
         "UNIT_TEST",
         "STREAMING",
         "REPLICATION_CGI",
         "SIPDB",
         "PROCESSMGR",
         "OS",
         "SIPXTAPI",
         "AUDIO",
         "CONFERENCE",
         "ODBC",
         "CDR",
         "RLS",
         "XMLRPC",
         "FSM",
         "NAT",
         "ALARM",
         "SAA",
         "UNKNOWN",
      } ;

      headers <<  facilityList[facility] << ":";

      return true;
    }
    public:
    std::string _hostName;
    std::string _processName;
  };

  class LogPriorityFilter
  {
  public:
    LogPriorityFilter()
    {
    }

    bool filter(int facility, int priority, const std::string& task, std::ostringstream& headers, std::string& message)
    {
      static const char* _priorityNames[8] =
      {
         "DEBUG",
         "INFO",
         "NOTICE",
         "WARNING",
         "ERR",
         "CRIT",
         "ALERT",
         "EMERG"
      };

      headers << _priorityNames[priority] << ":";
      return true;
    }
    public:
    std::string _hostName;
    std::string _processName;
  };

  class LogHostFilter
  {
  public:
    LogHostFilter()
    {
    }

    bool filter(int facility, int priority, const std::string& task, std::ostringstream& headers, std::string& message)
    {
      headers << _hostName << ":";
      return true;
    }
    public:
    std::string _hostName;
    std::string _processName;
   };

  class LogTaskNameFilter
  {
  public:
    LogTaskNameFilter()
    {
    }

    bool filter(int facility, int priority, const std::string& task, std::ostringstream& headers, std::string& message)
    {
      headers << task << ":";
      return true;
    }
    public:
    std::string _hostName;
    std::string _processName;
   };

  class LogThreadIdFilter
  {
  public:
    LogThreadIdFilter()
    {
    }

    bool filter(int facility, int priority, const std::string& task, std::ostringstream& headers, std::string& message)
    {
      headers << std::hex << pthread_self() << ":";
      return true;
    }
    public:
    std::string _hostName;
    std::string _processName;
  };

  class LogProcessNameFilter
  {
  public:
    LogProcessNameFilter()
    {
    }

    bool filter(int facility, int priority, const std::string& task, std::ostringstream& headers, std::string& message)
    {
      headers << _processName << ":";
      return true;
    }
    public:
    std::string _hostName;
    std::string _processName;
   };

  class LogEscapeFilter
  {
  public:
    LogEscapeFilter()
    {
    }
    bool filter(int facility, int priority, const std::string& task, std::ostringstream& headers, std::string& message)
    {
      std::string escaped;
      escaped.reserve(message.size());
      for (std::string::const_iterator iter = message.begin(); iter != message.end(); iter++)
      {
        if (*iter == '\\')
          escaped.append("\\\\");
        else if (*iter == '\r')
          escaped.append("\\r");
        else if (*iter == '\n')
          escaped.append("\\n");
        else if (*iter == '\"')
          escaped.append("\\\"");
        else
          escaped.append(1, *iter);
      }
      message = escaped;
      return true;
    }

  public:
    std::string _hostName;
    std::string _processName;
  };

  class NullFilter
  {
  public:
    NullFilter()
    {
    }

    bool filter(int facility, int priority, const std::string& task, std::ostringstream& headers, std::string& message)
    {
      return false;
    }
    public:
    std::string _hostName;
    std::string _processName;
  };

  template <
    typename TFilter_1 = NullFilter,
    typename TFilter_2 = NullFilter,
    typename TFilter_3 = NullFilter,
    typename TFilter_4 = NullFilter,
    typename TFilter_5 = NullFilter,
    typename TFilter_6 = NullFilter,
    typename TFilter_7 = NullFilter,
    typename TFilter_8 = NullFilter,
    typename TFilter_9 = NullFilter,
    typename TFilter_10 = NullFilter,
    typename TFilter_11 = NullFilter,
    typename TFilter_12 = NullFilter,
    typename TFilter_13 = NullFilter,
    typename TFilter_14 = NullFilter,
    typename TFilter_15 = NullFilter,
    typename TFilter_16 = NullFilter,
    typename TFilter_17 = NullFilter,
    typename TFilter_18 = NullFilter,
    typename TFilter_19 = NullFilter,
    typename TFilter_20 = NullFilter
  >
  class LogLevelFilter
  {
  public:
    enum LogLevel{ debug, information, notice, warning, error, critical, alert, emergency };
    
    LogLevelFilter(LogLevel level = notice) :
      _level(level)
    {
    }

    ~LogLevelFilter()
    {
    }

    bool filter(int facility, int priority, const std::string& task, std::ostringstream& headers, std::string& message)
    {
      std::map<int, int>::const_iterator facIter = _facilityLevel.find(facility);
      if (facIter != _facilityLevel.end() && priority < facIter->second)
        return false;
      else if (priority < _level)
        return false;

      if (_f1.filter(facility, priority,  task, headers, message))
        if (_f2.filter(facility, priority,  task, headers, message))
          if (_f3.filter(facility, priority,  task, headers, message))
            if (_f4.filter(facility, priority,  task, headers, message))
              if (_f5.filter(facility, priority,  task, headers, message))
                if (_f6.filter(facility, priority,  task, headers, message))
                  if (_f7.filter(facility, priority,  task, headers, message))
                    if (_f8.filter(facility, priority,  task, headers, message))
                      if (_f9.filter(facility, priority,  task, headers, message))
                        if (_f10.filter(facility, priority,  task, headers, message))
                          if (_f11.filter(facility, priority,  task, headers, message))
                            if (_f12.filter(facility, priority,  task, headers, message))
                              if (_f13.filter(facility, priority,  task, headers, message))
                                if (_f14.filter(facility, priority,  task, headers, message))
                                  if (_f15.filter(facility, priority,  task, headers, message))
                                    if (_f16.filter(facility, priority,  task, headers, message))
                                      if (_f17.filter(facility, priority,  task, headers, message))
                                        if (_f18.filter(facility, priority,  task, headers, message))
                                          if (_f19.filter(facility, priority,  task, headers, message))
                                            if (_f20.filter(facility, priority,  task, headers, message))
                                              return true;

      return true;
    }

    void setLevel(int level)
    {
      _level = (LogLevel)level;
    }

    void setLoggingPriorityForFacility(int facility, int level)
    {
      _facilityLevel[facility] = level;
    }

    int getLevel()
    {
      return _level;
    }

    void setHostName(const char* hostName)
    {
      _hostName = hostName;
      _f1._hostName = _hostName;
      _f2._hostName = _hostName;
      _f3._hostName = _hostName;
      _f4._hostName = _hostName;
      _f5._hostName = _hostName;
      _f6._hostName = _hostName;
      _f7._hostName = _hostName;
      _f8._hostName = _hostName;
      _f9._hostName = _hostName;
      _f10._hostName = _hostName;
      _f11._hostName = _hostName;
      _f12._hostName = _hostName;
      _f13._hostName = _hostName;
      _f14._hostName = _hostName;
      _f15._hostName = _hostName;
      _f16._hostName = _hostName;
      _f17._hostName = _hostName;
      _f18._hostName = _hostName;
      _f19._hostName = _hostName;
      _f20._hostName = _hostName;
    }

    void setProcessName(const char* processName)
    {
      _processName = processName;
      _f1._processName = _processName;
      _f2._processName = _processName;
      _f3._processName = _processName;
      _f4._processName = _processName;
      _f5._processName = _processName;
      _f6._processName = _processName;
      _f7._processName = _processName;
      _f8._processName = _processName;
      _f9._processName = _processName;
      _f10._processName = _processName;
      _f11._processName = _processName;
      _f12._processName = _processName;
      _f13._processName = _processName;
      _f14._processName = _processName;
      _f15._processName = _processName;
      _f16._processName = _processName;
      _f17._processName = _processName;
      _f18._processName = _processName;
      _f19._processName = _processName;
      _f20._processName = _processName;
    }

    public:
      std::string _hostName;
      std::string _processName;
      std::map<int, int> _facilityLevel;

  private:
    int _level;
    TFilter_1 _f1;
    TFilter_2 _f2;
    TFilter_3 _f3;
    TFilter_4 _f4;
    TFilter_5 _f5;
    TFilter_6 _f6;
    TFilter_7 _f7;
    TFilter_8 _f8;
    TFilter_9 _f9;
    TFilter_10 _f10;
    TFilter_11 _f11;
    TFilter_12 _f12;
    TFilter_13 _f13;
    TFilter_14 _f14;
    TFilter_15 _f15;
    TFilter_16 _f16;
    TFilter_17 _f17;
    TFilter_18 _f18;
    TFilter_19 _f19;
    TFilter_20 _f20;
  };

  template <typename T>
  class LoggerSingleton : boost::noncopyable
  {
  public:
    static T& instance()
    {
      static T singleton;
      return singleton;
    }
  };

  template <typename TFilter, typename TChannel>
  class LoggerBase : public LoggerSingleton<LoggerBase<TFilter, TChannel> >
  {
  public:
    typedef boost::shared_mutex mutex_read_write;
    typedef boost::shared_lock<boost::shared_mutex> mutex_read_lock;
    typedef boost::lock_guard<boost::shared_mutex> mutex_write_lock;
    typedef boost::function<std::string()> TaskCallBack;

    LoggerBase() :
      _flushRate(2)
    {
      _pChannel = new TChannel();
      _pFilter = new TFilter();
    }

    ~LoggerBase()
    {
      delete _pChannel;
      delete _pFilter;
    }

    bool open(const char* logFile)
    {
      mutex_write_lock lock(_mutex);
      return _pChannel->open(logFile);
    }

    bool reopen()
    {
      mutex_write_lock lock(_mutex);
      _pChannel->close();
      return _pChannel->open(_pChannel->path());
    }

    void setLevel(int level)
    {
      _pFilter->setLevel(level);
    }

    void setLogPriority(int priority)
    {
      _pFilter->setLevel(priority);
    }

    void setLoggingPriorityForFacility(int facility, int level)
    {
      _pFilter->setLoggingPriorityForFacility(facility, level);
    }

    void setHostName(const char* hostName)
    {
      _pFilter->setHostName(hostName);
    }

    void setProcessName(const char* processName)
    {
      _pFilter->setProcessName(processName);
    }

    //
    // This is the OsSysLog compatibility function
    //
    void add(int facility, int level, const char* format, ...)
    {
      char* buff;
      size_t needed = 1024;


      bool formatted = false;
      std::string message;
      for (int i = 0; !formatted && i < 3; i++)
      {
        va_list args;
        va_start(args, format);
        buff = (char*)::malloc(needed); /// Create a big enough buffer
        ::memset(buff, '\0', needed);
        int oldSize = needed;
        needed = vsnprintf(buff, needed, format, args) + 1;
        formatted = needed <= oldSize;
        if (formatted)
          message = buff;
        ::free(buff);
        va_end(args);
      }

      if (formatted)
        log(facility, level, message.c_str());
    }

    void log(int facility, int level, const char* msg)
    {
      std::string task;
      if (getCurrentTask)
        task = getCurrentTask();
      log(facility, level, task.c_str(), msg);
    }

    void log(int facility, int level, const std::string& taskName, const std::string& msg)
    {
      mutex_write_lock lock(_mutex);
      //
      // Create the header
      //
      std::ostringstream headers;
      std::string message(msg);
      if (_pFilter->filter(facility, level, taskName, headers, message))
      {
        //
        // Flush the log
        //
        headers << "\"" << message << "\"" << std::endl;
        std::string hdr(headers.str());
        _pChannel->write(hdr.c_str(), hdr.length());

        static unsigned long flushCount = 0;
        if (_flushRate && !(++flushCount % _flushRate))
          _pChannel->flush();
      }
    }

    void setFlushRate(unsigned flushRate)
    {
      _flushRate = flushRate;
    }

    void setCurrentTaskCallBack(TaskCallBack taskCallBack)
    {
      getCurrentTask = taskCallBack;
    }
  protected:

  private:
    TChannel* _pChannel;
    TFilter* _pFilter;
    mutex_read_write _mutex;
    unsigned _flushRate;
    TaskCallBack getCurrentTask;
  };

  typedef LogLevelFilter<
    LogTimeFilter,
    LogCounterFilter,
    LogFacilityFilter,
    LogPriorityFilter,
    LogHostFilter,
    LogTaskNameFilter,
    LogThreadIdFilter,
    LogProcessNameFilter,
    LogEscapeFilter> LogFilter;

  typedef LoggerBase<LogFilter, LogFileChannel> Logger;

  
  //
  // Macros
  //
  #define OS_LOG_PUSH(facility, priority, data) \
  { \
    std::ostringstream strm; \
    strm << data; \
    Os::Logger::log(facility, priority, strm.str().c_str()); \
  }

  #define OS_LOG_DEBUG(priority, data) OS_LOG_PUSH(Os::LogLevelFilter::debug, priority, data)
  #define OS_LOG_INFO(priority, data) OS_LOG_PUSH(Os::LogLevelFilter::information, priority, data)
  #define OS_LOG_NOTICE(priority, data) OS_LOG_PUSH(Os::LogLevelFilter::notice, priority, data)
  #define OS_LOG_WARNING(priority, data) OS_LOG_PUSH(Os::LogLevelFilter::warning, priority, data)
  #define OS_LOG_ERR(priority, data) OS_LOG_PUSH(Os::LogLevelFilter::error, priority, data)
  #define OS_LOG_CRIT(priority, data) OS_LOG_PUSH(Os::LogLevelFilter::critical, priority, data)
  #define OS_LOG_ALERT(priority, data) OS_LOG_PUSH(Os::LogLevelFilter::alert, priority, data)
  #define OS_LOG_EMERG(priority, data) OS_LOG_PUSH(Os::LogLevelFilter::emergency, priority, data)


} // Utl


#endif	/* LOGGER_H */

