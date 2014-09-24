#ifndef OSLOGGERHELPER_H
#define	OSLOGGERHELPER_H

#include <syslog.h>
#include <string>
#include "os/OsStatus.h"
#include "os/OsServerTask.h"
#include "os/OsDateTime.h"
#include "os/OsSocket.h"
#include "os/OsProcess.h"
#include "os/OsLogger.h"

namespace Os
{
  struct LoggerHelper
  {
    LoggerHelper()
    {
    }
    
    ~LoggerHelper()
    {
      //
      // Close the syslog
      //
      closelog ();
    }
    
    std::string getHostName()
    {
      if (hostName.empty())
      {
        UtlString h;
        OsSocket::getHostName(&h);
        if (!h.isNull())
          hostName = h.data();
      }
      return hostName;
    }

    std::string getCurrentTask()
    {
      OsTaskBase* pBase = OsTask::getCurrentTask();
      if (pBase)
        return pBase->getName().data();
      return std::string();
    }
    
    std::string getProcessName()
    {
      return processName;
    }

    static LoggerHelper& instance()
    {
      static LoggerHelper T;
      return T;
    }
    
    void initSysLog()
    {
      setlogmask (LOG_UPTO (LOG_NOTICE));
      openlog (0, LOG_PID | LOG_NDELAY | LOG_CONS, LOG_DAEMON );
      Logger::instance().setExternalLogger(boost::bind(&LoggerHelper::logPreview, this, _1, _2, _3, _4));
    }

    bool initialize(int priorityLevel, const char* path)
    {
      initSysLog();
      return Logger::instance().initialize<LoggerHelper>(priorityLevel, path, *this);
    }
    
    bool initialize(const char* path)
    {
      initSysLog();
      return Logger::instance().initialize<LoggerHelper>(path, *this);
    }
    
    bool logPreview(int facility, int level, const std::ostringstream& headers, std::string& message)
    {
      //
      // We dump emergency level to syslog
      //
      if (level == PRI_EMERG)
        syslog (LOG_EMERG, message.c_str());
      return false; // Tell the subsystem that we are not consuming the log by returning false
    }

    static bool parseLogString(const std::string& logData,
                             std::string& date,
                             std::string& eventCount,
                             std::string& facility,
                             std::string& priority,
                             std::string& hostName,
                             std::string& taskName,
                             std::string& taskId,
                             std::string& processId,
                             std::string& content)
    {
      enum parser_state
      {
        ParseDate,
        ParseEventCount,
        ParseFacility,
        ParsePriority,
        ParseHostName,
        ParseTaskName,
        ParseTaskId,
        ParseProcessId,
        ParseContent
      };

      parser_state state = ParseDate;
      for (std::string::const_iterator iter = logData.begin(); iter != logData.end(); iter++)
      {
        switch(state)
        {
        case ParseDate:
          if (*iter != ':')
            date += *iter;
          else
            state = ParseEventCount;
          break;
        case ParseEventCount:
          if (*iter != ':')
            eventCount += *iter;
          else
            state = ParseFacility;
          break;
        case ParseFacility:
          if (*iter != ':')
            facility += *iter;
          else
            state = ParsePriority;
          break;
        case ParsePriority:
          if (*iter != ':')
            priority += *iter;
          else
            state = ParseHostName;
          break;
        case ParseHostName:
          if (*iter != ':')
            hostName += *iter;
          else
            state = ParseTaskName;
          break;
        case ParseTaskName:
          if (*iter != ':')
            taskName += *iter;
          else
            state = ParseTaskId;
          break;
        case ParseTaskId:
          if (*iter != ':')
            taskId += *iter;
          else
            state = ParseProcessId;
          break;
        case ParseProcessId:
          if (*iter != ':')
            processId += *iter;
          else
            state = ParseContent;
          break;
        case ParseContent:
          if (*iter != ':')
            content += *iter;
          break;
        }
      }
      return state == ParseContent;
    }

    std::string hostName;
    std::string processName;
  };
}

#endif	/* OSLOGGERHELPER_H */

