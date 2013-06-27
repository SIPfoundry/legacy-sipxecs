#ifndef OSLOGGERHELPER_H
#define	OSLOGGERHELPER_H

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
    void setFilterNames(const std::string& filterNames)
    {
      this->filterNames = filterNames;
    }

    void setProcessName(const std::string& processName)
    {
      this->processName = processName;
    }

    void setHostName(const std::string& hostName)
    {
      this->hostName = hostName;
    }

    std::string getFilterNames()
    {
      return filterNames;
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

    bool initialize(int priorityLevel, const char* path)
    {
      return Logger::instance().initialize<LoggerHelper>(priorityLevel, path, *this);
    }
    
    bool initialize(const char* path)
    {
      return Logger::instance().initialize<LoggerHelper>(path, *this);
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
    std::string filterNames;
  };
}

#endif	/* OSLOGGERHELPER_H */

