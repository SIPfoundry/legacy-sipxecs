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
    
    void initAlarmLog(const char* alarmLog)
    {
      if (!alarmLog)
        return;
      
      std::ostringstream path;
      path << alarmLog;
      _alarmLog.open(path.str().c_str());
      _alarmLogRotate.start(&_alarmLog);
    }

    bool initialize(int priorityLevel, const char* path, const char* alarmLog = 0)
    {
      initSysLog();
      initAlarmLog(alarmLog);
      return Logger::instance().initialize<LoggerHelper>(priorityLevel, path, *this);
    }
    
    bool initialize(const char* path, const char* alarmLog = 0)
    {
      initSysLog();
      initAlarmLog(alarmLog);
      return Logger::instance().initialize<LoggerHelper>(path, *this);
    }
    

    static UtlString unescape(const UtlString& source)
    {
       UtlString    results ;
       const char* pStart = source.data() ;
       const char* pTraverse = pStart ;
       const char* pLast = pStart ;
       UtlBoolean   bLastWasEscapeChar = false;

       while (*pTraverse)
       {
          if (bLastWasEscapeChar)
          {
             switch (*pTraverse)
             {
                case '\\':
                case '"':
                   if (pLast < pTraverse)
                   {
                      results.append(pLast, pTraverse-pLast-1);
                   }
                   pLast = pTraverse + 1 ;
                   results.append(*pTraverse) ;
                   break ;
                case 'r':
                   if (pLast < pTraverse)
                   {
                      results.append(pLast, pTraverse-pLast-1);
                   }
                   pLast = pTraverse + 1 ;
                   results.append("\r") ;
                   break ;
                case 'n':
                   if (pLast < pTraverse)
                   {
                      results.append(pLast, pTraverse-pLast-1);
                   }
                   pLast = pTraverse + 1 ;
                   results.append("\n") ;
                   break;
                default:
                   // Invalid/Illegal Escape Character
                   break ;
             }
             bLastWasEscapeChar = false ;
          }
          else
          {
             if (*pTraverse == '\\')
             {
                bLastWasEscapeChar = true ;
             }
          }

          pTraverse++ ;
       }

       // if nothing to escape, short-circuit
       if (pLast == pStart)
       {
          return source ;
       }
       else if (pLast < pTraverse)
       {
          results.append(pLast, (pTraverse-1)-pLast);
       }

       return results ;
    }

    bool logPreview(int facility, int level, const std::ostringstream& headers, std::string& message)
    {
      //
      // Dump critical->emergency log entries to alarms 
      //
      if (level >= PRI_CRIT)
      {
        _alarmLogRotate.wakeup();
        
        if (_alarmLog.is_open())
        {
          std::ostringstream log;
          log << headers.str() << "\"" << message << "\"" << std::endl;
          _alarmLog.write(log.str().c_str(), log.str().size());
          _alarmLog.flush();
        }
        //
        // We dump emergency level to syslog as well
        //
        if (level == PRI_EMERG)
          syslog (LOG_EMERG, message.c_str());
      }
      
      return false; // Tell the subsystem that we are not consuming the log by returning false
    }

    //:Parses a log string into its parts.
    static OsStatus parseLogString(const char *szSource,
                                      UtlString& date,
                                      UtlString& eventCount,
                                      UtlString& facility,
                                      UtlString& priority,
                                      UtlString& hostname,
                                      UtlString& taskname,
                                      UtlString& taskId,
                                      UtlString& processId,
                                      UtlString& content)
    {
       #define PS_DATE         0
       #define PS_EVENTCOUNT   1
       #define PS_FACILITY     2
       #define PS_PRIORITY     3
       #define PS_HOSTNAME     4
       #define PS_TASKNAME     5
       #define PS_TASKID       6
       #define PS_PROCESSID    7
       #define PS_CONTENT      8

       const char* pTraverse = szSource ;  // Traverses the source string
       UtlBoolean   bWithinQuote = FALSE;   // Are we within a quoted string?
       UtlBoolean   bEscapeNext = FALSE;    // The next char is an escape char.
       int         iParseState ;           // What are we parsing (PS_*)

       // Clean all of the passed objects
       date.remove(0) ;
       eventCount.remove(0) ;
       facility.remove(0) ;
       priority.remove(0) ;
       hostname.remove(0) ;
       taskname.remove(0) ;
       processId.remove(0) ;
       content.remove(0) ;

       // Loop through the source string and add characters to the appropriate
       // data object
       iParseState = PS_DATE ;
       while (*pTraverse)
       {
          switch (*pTraverse)
          {
             case ':':
                if (!bWithinQuote)
                {
                   iParseState++ ;
                   pTraverse++ ;
                   continue ;
                }
                break ;
             case '"':
                if (!bEscapeNext)
                {
                   bWithinQuote = !bWithinQuote;
                   pTraverse++ ;
                   continue ;
                }
                break ;
             case '\\':
                bEscapeNext = true ;
                break ;
          default:
                break;
          }

          switch (iParseState)
          {
             case PS_DATE:
                date.append(*pTraverse) ;
                break ;
             case PS_EVENTCOUNT:
                eventCount.append(*pTraverse) ;
                break ;
             case PS_FACILITY:
                facility.append(*pTraverse) ;
                break ;
             case PS_PRIORITY:
                priority.append(*pTraverse) ;
                break ;
             case PS_HOSTNAME:
                hostname.append(*pTraverse) ;
                break ;
             case PS_TASKNAME:
                taskname.append(*pTraverse) ;
                break ;
             case PS_TASKID:
                taskId.append(*pTraverse) ;
                break ;
             case PS_PROCESSID:
                processId.append(*pTraverse) ;
                break ;
             case PS_CONTENT:
                content.append(*pTraverse) ;
                break ;
             default:
                break;
          }

          pTraverse++ ;
       }

       content = unescape(content) ;

       return OS_SUCCESS ;
    }

    std::string hostName;
    std::string processName;
    
  protected:
    LogFileChannel _alarmLog;
    LogRotateStrategy<LogFileChannel> _alarmLogRotate;
  };
}

#endif	/* OSLOGGERHELPER_H */

