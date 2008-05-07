/***********************************************************************
 ***********************************************************************
 *
 * 
 *
 * Implementation of the OSBlog functions defined in OSBlog.h, see
 * that header for details
 *
 ***********************************************************************
 **********************************************************************/


/****************License************************************************
 *
 * Copyright 2000-2001.  SpeechWorks International, Inc.  
 *
 * Use of this software is subject to notices and obligations set forth
 * in the SpeechWorks Public License - Software Version 1.1 which is
 * included with this software.
 *
 * SpeechWorks is a registered trademark, and SpeechWorks Here, 
 * DialogModules and the SpeechWorks logo are trademarks of SpeechWorks 
 * International, Inc. in the United States and other countries. 
 * 
 ************************************************************************/

static const char *rcsid = 0 ? (char *) &rcsid :
"";

// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8

#include <cstdio>
#include <cstring>                  // For memset()
#include <ctime>                    // For time_t
#include <ctype.h>                  // For isspace()
#include <wctype.h>                 // For iswalpha()

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sys/timeb.h>              // For _ftime()
#else
#include <time.h>                   // For gettimeofday()
#include <sys/time.h>
#include <unistd.h>
#endif

#define OSBLOG_EXPORTS
#include "OSBlog.h"                 // Header for these functions
#include "VXItrd.h"                 // for VXItrdMutex
#include "SBlogOSUtils.h"

#define MAX_LOG_BUFFER  4096

#ifndef MODULE_PREFIX
#define MODULE_PREFIX  COMPANY_DOMAIN L"."
#endif

// Global variables
static bool gblInitialized = false;
static VXItrdMutex *gblLogMutex = NULL;
static FILE *gblLogFile = NULL;
static bool gblLogToStdout = false;

// Convert wide to narrow characters
#define w2c(w) (((w) & 0xff00)?'\277':((unsigned char) ((w) & 0x00ff)))


/**********************************************************************/
// Log utility classes
/**********************************************************************/

const VXIint  SEP_LEN     = 1;
const VXIchar KEY_SEP     = L'=';
const VXIchar ENTRY_SEP   = L'|';
const VXIchar ESCAPE      = L'\\';

class LogEntry {
public:
  LogEntry();
  virtual ~LogEntry() {}

  VXIlogResult Append(const VXIchar*, VXIunsigned n);
  VXIlogResult AppendVa(const VXIchar *format, va_list args);
  VXIlogResult AppendKeyValueVa(const VXIchar *format, va_list args);

  void operator+= (const VXIchar *val) { Append(val, 0); }
  void operator+= (const VXIchar val){
    if(cur_size_ < MAX_LOG_BUFFER){
      entry_[cur_size_++] =  val;
    }
    entry_[cur_size_]=L'\0';
    return ;
  }
  void operator+= (const char*);
  void operator+= (long int);
  void operator+= (VXIint32);
  void operator+= (VXIunsigned);

  void AddKeySep(){
    (*this) += KEY_SEP;
  }
  void AddEntrySep(){
    (*this) += ENTRY_SEP;
  }
  void Terminate() {
    if(cur_size_ == MAX_LOG_BUFFER)
      cur_size_--;
    entry_[cur_size_++] ='\n';
    entry_[cur_size_] = '\0';
  }

  const wchar_t *Entry()const {return entry_;}
  
  VXIunsigned size() const {
    return cur_size_;
  };

protected:
  unsigned int cur_size_;
  wchar_t entry_[MAX_LOG_BUFFER+1];
};


LogEntry::LogEntry() : cur_size_(0)
{
  time_t  timestamp;
  VXIunsigned timestampMsec;
  char timestampStr[128];
  SBlogGetTime (&timestamp, &timestampMsec);
  SBlogGetTimeStampStr(timestamp,timestampMsec,timestampStr);
  (*this) += timestampStr;

  long userTime;
  long kernelTime;
  SBlogGetCPUTimes(&userTime,&kernelTime);
  this->AddEntrySep();
  (*this) += "TUCPU";
  this->AddKeySep();
  (*this) += userTime;
  this->AddEntrySep();

  (*this) += "TKCPU";
  this->AddKeySep();
  (*this) += kernelTime;
}

VXIlogResult
LogEntry::Append (const VXIchar* val, VXIunsigned n)
{
  unsigned int i = 0;
  if (n <= 0)
    n = (VXIunsigned)wcslen(val);

  // strip leading and trailing whitespace
  while ((n > 0) && (iswspace(val[n-1])))
    n--;
  while ((i < n) && (iswspace(val[i])))
    i++;

  while((cur_size_ < MAX_LOG_BUFFER) &&
	(i < n)){
    if (iswspace(val[i])) {
      entry_[cur_size_++] = L' ';
    } else {
      if ((val[i] == KEY_SEP) || (val[i] == ENTRY_SEP) || 
	  (val[i] == ESCAPE))
	entry_[cur_size_++] = ESCAPE;

      entry_[cur_size_++] = val[i];
    }
    i++;
  }
  entry_[cur_size_]=L'\0';

  return VXIlog_RESULT_SUCCESS;
}

VXIlogResult 
LogEntry::AppendVa(const VXIchar *format, va_list args)
{
  if (! format)
    return VXIlog_RESULT_SUCCESS;

  SBlogVswprintf(&entry_[cur_size_], MAX_LOG_BUFFER - cur_size_, format, args);
  cur_size_ += wcslen(&entry_[cur_size_]);

  return VXIlog_RESULT_SUCCESS;
}

VXIlogResult 
LogEntry::AppendKeyValueVa(const VXIchar *format, va_list args)
{
  if (! format)
    return VXIlog_RESULT_SUCCESS;

  // First create a modified format string that will properly delimit
  // the key/value pairs
  VXIlogResult rc = VXIlog_RESULT_SUCCESS;
  bool hadFreeformText = false;
  int replacementStart = -1, fieldCount = 0;
  size_t resultFormatLen = 0;
  VXIchar resultFormat[MAX_LOG_BUFFER];
  resultFormat[0] = L'\0';

  for (int i = 0; (format[i] != L'\0') && (rc == VXIlog_RESULT_SUCCESS); i++) {
    if (format[i] == '%') {
      if (replacementStart > -1)
	replacementStart = -1; // double %%
      else
	replacementStart = i;
    } else if ((replacementStart > -1) && (iswalpha(format[i])) &&
	       (format[i] != L'l') && (format[i] != L'L') && 
	       (format[i] != L'h')) {
      if ((fieldCount % 2 == 0) && (format[i] != L's') && 
	  (format[i] != L'S')) {
	// Keys must be a %s or %S, truncate from here
	rc = VXIlog_RESULT_NON_FATAL_ERROR;
      } else {
	// Insert the replacement expression and the seperator
	int index = resultFormatLen;
	resultFormatLen += (i - replacementStart) + 1 + SEP_LEN;
	if (resultFormatLen < MAX_LOG_BUFFER) {
	  wcsncpy(&resultFormat[index], &format[replacementStart],
		  (i - replacementStart) + 1);
	  index += (i - replacementStart) + 1;
	  if (fieldCount % 2 == 0)
	    resultFormat[index++] = KEY_SEP;
	  else if (format[i + 1] != L'\0')
	    resultFormat[index++] = ENTRY_SEP;
	  resultFormat[index] = L'\0';
	} else {
	  // Overflow, truncate the format string from here
	  rc = VXIlog_RESULT_NON_FATAL_ERROR;
	}
	
	replacementStart = -1;
	fieldCount++;
      }
    } else if (replacementStart == -1) {
      // Shouldn't have free-form text, skip it. Proceeding allows us
      // to gracefully handle things like "%s0x%p".
      hadFreeformText = true;
    }
  }

  // if key/value is not even truncate the field and return an error,
  // but proceed with the other fields. If there was free form text,
  // we skipped it and return an error, but proceed with logging.
  if (fieldCount % 2 != 0) {
    rc = VXIlog_RESULT_NON_FATAL_ERROR;
    fieldCount--;
  } else if (hadFreeformText) {
    rc = VXIlog_RESULT_NON_FATAL_ERROR;
  }

  // Now create the final output 
  SBlogVswprintf(&entry_[cur_size_], MAX_LOG_BUFFER - cur_size_, 
		 resultFormat, args);
  cur_size_ += wcslen(&entry_[cur_size_]);

  return rc;
}

void 
LogEntry::operator+= (const char* val)
{
  unsigned int i = 0;
  unsigned int n = strlen(val);

  // strip leading and trailing whitespace
  while ((n > 0) && (isspace(val[n-1])))
    n--;
  while ((i < n) && (isspace(val[i])))
    i++;

  while((cur_size_ < MAX_LOG_BUFFER) &&
	(i < n)){
    if (isspace(val[i])) {
      entry_[cur_size_++] = L' ';
    } else {
      if ((val[i] == KEY_SEP) || (val[i] == ENTRY_SEP) || 
	  (val[i] == ESCAPE))
	entry_[cur_size_++] = ESCAPE;

      entry_[cur_size_++] = (VXIchar) val[i];
    }
    i++;
  }
  entry_[cur_size_]=L'\0';
  return ;
}


void
LogEntry::operator+= (VXIint32 val)
{
  // how can temp be made the right size to start?
  char temp[128];
  sprintf(temp,"%d",val);
  (*this) += temp;
  return ;
}

void
LogEntry::operator+= (long int val)
{
  // how can temp be made the right size to start?
  char temp[128];
  sprintf(temp,"%ld",val);
  (*this) += temp;
  return ;
}

void
LogEntry::operator+= (VXIunsigned val)
{
  char temp[128];
  sprintf(temp,"%d",val);
  (*this) += temp;
  return ;
}


/**********************************************************************/
// Logging class
/**********************************************************************/

struct myAPI {
  OSBlogInterface intf;
  void *impl_;
};

#ifdef setbit
#undef setbit
#endif

class OSBlog {
public:
  OSBlog(VXIint channelNum);
  ~OSBlog();
  
  bool DiagnosticIsEnabled(VXIunsigned tagID);

  VXIlogResult DiagnosticLog(VXIunsigned    tagID,
                             const VXIchar* subtag,
                             const VXIchar* format,
                             va_list        args);
  
  VXIlogResult EventLog(VXIunsigned         eventID,
                        const VXIchar*      format,
                        va_list             args);

  VXIlogResult ErrorLog(const VXIchar*      moduleName,
                        VXIunsigned         errorID,
                        const VXIchar*      format,
                        va_list             args);  

  VXIlogResult ControlDiagnosticTag(VXIunsigned tagID,
                                    VXIbool     state);

  VXIlogResult Flush(const char   *logFileName,
				             VXIbool       logToStdout);

private:
private:
  // Internal methods
  static unsigned testbit(unsigned char num, int bitpos);
  static void setbit(unsigned char *num, int bitpos);
  static void clearbit(unsigned char *num, int bitpos);

  int Convert2Index(VXIunsigned tagID,
		    VXIunsigned *index, 
		    VXIunsigned *bit_pos) const;

  VXIlogResult WriteEntry(const LogEntry &entry, bool logToStdout = true);

protected:
  VXIint channelNum;
  VXItrdMutex *callbackLock;
  unsigned char TagIDs[LOG_MAX_TAG];
};


OSBlog::OSBlog(VXIint chNum) : channelNum(chNum)
{
  // reset TAG ID range
  memset(TagIDs,0,LOG_MAX_TAG);
  VXItrdMutexCreate(&callbackLock); 
}


OSBlog::~OSBlog() 
{
  VXItrdMutexDestroy(&callbackLock);
}

/**
 * testbit
 * testbit returns the value of the given bit
 * 1 is set, 0 is clear
 */
unsigned OSBlog::testbit(unsigned char num, int bitpos)
{ 
  return (num >> bitpos) & ~(~0 << 1); 
}

/**
 * setbit sets a given bit
 */
void OSBlog::setbit(unsigned char *num, int bitpos)
{
  *num |= (1 << bitpos);
}

/**
 * clearbit clears a given bit
 */
void OSBlog::clearbit(unsigned char *num, int bitpos)
{
  *num &= ~(1 << bitpos);
}


int OSBlog::Convert2Index(VXIunsigned tagID,
			  VXIunsigned *index, 
			  VXIunsigned *bit_pos) const
{
  if (tagID < 0) return 0;
  // retrieving index for char array
  *index = tagID/8;  // 8 bits per char
  // check for overflow TAG ID
  if (*index >= LOG_MAX_TAG)
    return 0;
  // retrieving bit position (bit range from 0-7)
  *bit_pos = tagID%7;
  return 1;  // done
}


VXIlogResult OSBlog::ControlDiagnosticTag(VXIunsigned tagID,
                                         VXIbool state)
{
  VXIunsigned bindex, bpos;
  if(!Convert2Index(tagID, &bindex, &bpos))
    return VXIlog_RESULT_INVALID_ARGUMENT;

  if(state)
    setbit(&TagIDs[bindex], bpos);
  else
    clearbit(&TagIDs[bindex], bpos);
    
  return VXIlog_RESULT_SUCCESS;
}


bool OSBlog::DiagnosticIsEnabled(VXIunsigned tagID)
{
  VXIunsigned bindex, bpos;
  if (!Convert2Index(tagID, &bindex, &bpos))
    return false;

  // if this tag is not turned on, just return
  if (!testbit(TagIDs[bindex],bpos))
    return false;

  return true;
}


VXIlogResult OSBlog::DiagnosticLog(VXIunsigned     tagID,
				   const VXIchar*  subtag,
				   const VXIchar*  format,
				   va_list         args)
{
  VXIunsigned bindex, bpos;
  if (!Convert2Index(tagID, &bindex, &bpos))
    return VXIlog_RESULT_INVALID_ARGUMENT;

  if (! format)
    return VXIlog_RESULT_SUCCESS;

  // if this tag is not turned on, just return
  if (!testbit(TagIDs[bindex],bpos))
    return VXIlog_RESULT_SUCCESS;

  if (subtag == NULL)
    subtag = L"";
  
  // Output the log message
  LogEntry entry;
  entry.AddEntrySep();
  entry += channelNum;
  entry.AddEntrySep();
  entry += tagID;
  entry.AddEntrySep();
  entry += subtag;
  entry.AddEntrySep();
  VXIlogResult rc;
  if (args == 0)
     rc = entry.Append(format, wcslen(format));
  else
     rc = entry.AppendVa(format, args);
  entry.Terminate();

  VXIlogResult rc2 = WriteEntry(entry);
  if (rc2 != VXIlog_RESULT_SUCCESS)
    rc = rc2;
  
  return rc;
}


VXIlogResult OSBlog::EventLog(VXIunsigned      eventID,
			      const VXIchar*   format,
			      va_list          args)
{
  // Output the log message
  LogEntry entry;
  entry.AddEntrySep();
  entry += channelNum;
  entry.AddEntrySep();
  entry += L"EVENT";
  entry.AddEntrySep();
  entry += eventID;
  entry.AddEntrySep();
  VXIlogResult rc = entry.AppendKeyValueVa(format, args);
  entry.Terminate();

  // Don't write events to stdout
  VXIlogResult rc2 = WriteEntry(entry, false);
  if (rc2 != VXIlog_RESULT_SUCCESS)
    rc = rc2;

  return rc;
}


VXIlogResult OSBlog::ErrorLog(const VXIchar*   moduleName,
			      VXIunsigned      errorID,
			      const VXIchar*   format,
			      va_list          args)
{
  const VXIchar *finalModuleName = 
    (moduleName && moduleName[0] ? moduleName : L"UNKNOWN");

  // Output the log message
  LogEntry entry;
  entry.AddEntrySep();
  entry += channelNum;
  entry.AddEntrySep();
  entry += finalModuleName;
  entry.AddEntrySep();
  entry += errorID;
  entry.AddEntrySep();
  VXIlogResult rc = entry.AppendKeyValueVa(format, args);
  entry.Terminate();

  VXIlogResult rc2 = WriteEntry(entry);
  if (rc2 != VXIlog_RESULT_SUCCESS)
    rc = rc2;

  // Want to warn the caller that NULL moduleName really isn't OK, but
  // logged it anyway
  if ((! moduleName) || (! moduleName[0]))
    rc = VXIlog_RESULT_INVALID_ARGUMENT;
  
  return rc;
}
  

VXIlogResult OSBlog::WriteEntry(const LogEntry &entry, bool logToStdout)
{
  if (((! gblLogFile) && (! gblLogToStdout)) || (entry.size() < 1))
    return VXIlog_RESULT_SUCCESS;

  // Convert to narrow characters
  unsigned int i;
  unsigned int n = entry.size();
  const wchar_t *ptr = entry.Entry();
  char outbuf[MAX_LOG_BUFFER];
  for(i=0; i<n; i++)
    outbuf[i] = w2c(ptr[i]);
  outbuf[i] = '\0';

  // Lock and write out the log entry
  if (VXItrdMutexLock(gblLogMutex) != VXItrd_RESULT_SUCCESS)
    return VXIlog_RESULT_SYSTEM_ERROR;

  VXIlogResult rc = VXIlog_RESULT_SUCCESS;
  if (gblLogFile) {
    if (fwrite(outbuf, sizeof(char), n, gblLogFile) < 1) {
      // should disable logging.
      rc = VXIlog_RESULT_IO_ERROR;
    } else {
      // to ensure we don't lose log lines on a crash/abort
      fflush(gblLogFile);
    }
  }

  if (gblLogToStdout && logToStdout &&
      (fwrite(outbuf, sizeof(char), n, stdout) < 1 || fflush(stdout) != 0))
    rc = VXIlog_RESULT_IO_ERROR;

  if (VXItrdMutexUnlock(gblLogMutex) != VXItrd_RESULT_SUCCESS)
    rc = VXIlog_RESULT_SYSTEM_ERROR;

  return rc;
}

/***********************************************************************/

VXIlogResult OSBlogControlDiagnosticTag(OSBlogInterface *pThis,
					VXIunsigned tagID,
					VXIbool     state)
{
  if (pThis == NULL)
    return VXIlog_RESULT_INVALID_ARGUMENT;
  myAPI *temp = (myAPI *) pThis; 
  OSBlog *me = (OSBlog *)temp->impl_;
  return (me->ControlDiagnosticTag(tagID, state));
}


/**********************************************************************
*/

OSBLOG_API VXIint32 OSBlogGetVersion(void)
{
  return VXI_CURRENT_VERSION;
}

OSBLOG_API const VXIchar* OSBlogGetImplementationName(void)
{
  static const VXIchar IMPLEMENTATION_NAME[] = COMPANY_DOMAIN L".OSBlog";
  return IMPLEMENTATION_NAME;
}


OSBLOG_API VXIbool OSBlogDiagnosticIsEnabled(VXIlogInterface * pThis,
					     VXIunsigned tagID)
{
  if (pThis == NULL)
    return FALSE;

  myAPI *temp = (myAPI *) pThis;
  OSBlog *me = (OSBlog *) temp->impl_;

  if (!me->DiagnosticIsEnabled(tagID)) return FALSE;
  return TRUE;
}


OSBLOG_API VXIlogResult OSBlogDiagnostic(VXIlogInterface*        pThis,
					 VXIunsigned             tagID,
					 const VXIchar*          subtag,
					 const VXIchar*          format,
					 ...)
{
  if (pThis == NULL)
    return VXIlog_RESULT_INVALID_ARGUMENT;

  myAPI *temp = (myAPI *) pThis;
  OSBlog *me = (OSBlog *) temp->impl_;
  va_list args;
  va_start(args, format);

  VXIlogResult ret = me->DiagnosticLog(tagID, subtag, format, args);
  va_end(args);

  return ret;
}

OSBLOG_API VXIlogResult OSBlogVDiagnostic(VXIlogInterface*        pThis,
					  VXIunsigned             tagID,
					  const VXIchar*          subtag,
					  const VXIchar*          format,
					  va_list                 vargs)
{
  if (pThis == NULL)
    return VXIlog_RESULT_INVALID_ARGUMENT;

  myAPI *temp = (myAPI *)pThis;
  OSBlog *me = (OSBlog *)temp->impl_;
  VXIlogResult ret = me->DiagnosticLog(tagID, subtag, format, vargs);

  return ret;
}


OSBLOG_API VXIlogResult OSBlogEvent(VXIlogInterface*        pThis,
				    VXIunsigned             eventID,
				    const VXIchar*          format,
				    ...)
{
  VXIlogResult ret = VXIlog_RESULT_SUCCESS;

  if (pThis == NULL)
    return VXIlog_RESULT_INVALID_ARGUMENT;

#ifdef VXILOG_EVENTS_ENABLED
  myAPI *temp = (myAPI*)pThis;
  OSBlog *me = (OSBlog *)temp->impl_;
  va_list args;
  va_start(args, format);
  ret = me->EventLog(eventID, format, args);
  va_end(args);
#endif
  return ret;
}


OSBLOG_API VXIlogResult OSBlogVEvent(VXIlogInterface*        pThis,
				     VXIunsigned             eventID,
				     const VXIchar*          format,
				     va_list                 vargs)
{
  VXIlogResult ret = VXIlog_RESULT_SUCCESS;

  if (pThis == NULL)
    return VXIlog_RESULT_INVALID_ARGUMENT;

#ifdef VXILOG_EVENTS_ENABLED
  myAPI *temp = (myAPI*)pThis;
  OSBlog *me = (OSBlog *)temp->impl_;
  ret = me->EventLog(eventID, format, vargs);
#endif
  return ret;
}

OSBLOG_API VXIlogResult OSBlogError(VXIlogInterface*        pThis,
				    const VXIchar*          moduleName,
				    VXIunsigned             errorID,
				    const VXIchar*          format,
				    ...)
{
  if (pThis == NULL)
    return VXIlog_RESULT_INVALID_ARGUMENT;

  myAPI *temp = (myAPI*)pThis;
  OSBlog *me = (OSBlog *)temp->impl_;
  va_list args;
  va_start(args, format);
  VXIlogResult ret = me->ErrorLog(moduleName, errorID, format, args);
  va_end(args);

  return ret;
}


OSBLOG_API VXIlogResult OSBlogVError(VXIlogInterface*        pThis,
				     const VXIchar*          moduleName,
				     VXIunsigned             errorID,
				     const VXIchar*          format,
				     va_list                 vargs)
{
  if (pThis == NULL)
    return VXIlog_RESULT_INVALID_ARGUMENT;

  myAPI *temp = (myAPI*)pThis;
  OSBlog *me = (OSBlog *)temp->impl_;
  VXIlogResult ret = me->ErrorLog(moduleName, errorID, format, vargs);

  return ret;
}

// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8

/**
 * Global platform initialization of OSBlog
 *
 * @param  logFileName      Name of the file where diagnostic, error, and 
 *                          event information will be written. Pass NULL 
 *                          to disable logging to a file.
 * @param  logToStdout      TRUE to log diagnostic and error messages to
 *                          standard out, FALSE to disable. Event reports
 *                          are never logged to standard out.
 *
 * @result VXIlogResult 0 on success
 */
OSBLOG_API VXIlogResult OSBlogInit(const char   *logFileName,
				   VXIbool       logToStdout)
{
  if (gblInitialized == true)
    return VXIlog_RESULT_FATAL_ERROR;

  // Create the log mutex
  if (VXItrdMutexCreate(&gblLogMutex) != VXItrd_RESULT_SUCCESS)
    return VXIlog_RESULT_SYSTEM_ERROR;

  // Open the log file
  if ((logFileName) && (logFileName[0])) {
    gblLogFile = fopen(logFileName, "a");
    if (! gblLogFile)
      return VXIlog_RESULT_IO_ERROR;
  }

  gblInitialized = true;
  gblLogToStdout = (logToStdout ? true : false);
  return VXIlog_RESULT_SUCCESS;
}


/**
 * Global platform shutdown of Log
 *
 * @result VXIlogResult 0 on success
 */
OSBLOG_API VXIlogResult OSBlogShutDown(void)
{
  if (gblInitialized == false)
    return VXIlog_RESULT_FATAL_ERROR;

  // Close the log file
  if (gblLogFile) {
    fclose(gblLogFile);
    gblLogFile = NULL;
  }

  // Destroy the log mutex
  if (gblLogMutex)
    VXItrdMutexDestroy(&gblLogMutex);

  gblInitialized = false;
  return VXIlog_RESULT_SUCCESS;
}

  /**
   * Called to flush the log file periodically.
   *    
   * @return VXIlog_RESULT_SUCCESS on success
   */

OSBLOG_API VXIlogResult OSBlogFlush(VXIlogInterface* pThis,
                                    const char   *logFileName,
				                            VXIbool       logToStdout)
{
  // Lock and write out the log entry
  if (VXItrdMutexLock(gblLogMutex) != VXItrd_RESULT_SUCCESS)
    return VXIlog_RESULT_SYSTEM_ERROR;

  // Close the log file
  if (gblInitialized && gblLogFile) {
    fflush(gblLogFile);
    fclose(gblLogFile);
  }

  // Open the log file
  if ((logFileName) && (logFileName[0])) {
    gblLogFile = fopen(logFileName, "a");
    if (! gblLogFile)
      return VXIlog_RESULT_IO_ERROR;
  }

  gblLogToStdout = (logToStdout ? true : false);

  if (VXItrdMutexUnlock(gblLogMutex) != VXItrd_RESULT_SUCCESS)
    return VXIlog_RESULT_SYSTEM_ERROR;

  return VXIlog_RESULT_SUCCESS;
}



/**
 * Create a new log service handle
 *
 * @param channelNum      [IN] Logical channel number
 *
 * @result VXIlogResult 0 on success 
 */
OSBLOG_API VXIlogResult OSBlogCreateResource(VXIint            channelNum,
					     VXIlogInterface **log)
{
  if (gblInitialized == false)
    return VXIlog_RESULT_FATAL_ERROR;

  if (log == NULL)
    return VXIlog_RESULT_INVALID_ARGUMENT;

  myAPI *temp = new myAPI;
  if (temp == NULL)
    return VXIlog_RESULT_OUT_OF_MEMORY;
  memset (temp, 0, sizeof (myAPI));
  
  OSBlog *me = new OSBlog(channelNum);
  if (me == NULL) {
    delete temp;
    temp = NULL;
    return VXIlog_RESULT_OUT_OF_MEMORY;
  }
  
  // Initialize the VXIlogInterface function pointers
  temp->intf.vxilog.GetVersion            = OSBlogGetVersion;
  temp->intf.vxilog.GetImplementationName = OSBlogGetImplementationName;
  temp->intf.vxilog.Error                 = OSBlogError;
  temp->intf.vxilog.VError                = OSBlogVError;
  temp->intf.vxilog.Diagnostic            = OSBlogDiagnostic;
  temp->intf.vxilog.VDiagnostic           = OSBlogVDiagnostic;
  temp->intf.vxilog.DiagnosticIsEnabled   = OSBlogDiagnosticIsEnabled;
  temp->intf.vxilog.Event                 = OSBlogEvent;
  temp->intf.vxilog.VEvent                = OSBlogVEvent;
  temp->intf.vxilog.Flush                 = OSBlogFlush;
  
  // Initialize the OSBlogInterface functions
  temp->intf.ControlDiagnosticTag = OSBlogControlDiagnosticTag;
  temp->impl_ = (void*)me;

  // Return the object
  *log = &temp->intf.vxilog;
  return VXIlog_RESULT_SUCCESS;
}


/**
 * Destroy the interface and free internal resources
 *
 * @result VXIlogResult 0 on success 
 */
OSBLOG_API VXIlogResult OSBlogDestroyResource(VXIlogInterface **log)
{
  if (gblInitialized == false)
    return VXIlog_RESULT_FATAL_ERROR;

  if ((log == NULL) || (*log == NULL))
    return VXIlog_RESULT_INVALID_ARGUMENT;

  // Delete the object
  myAPI *temp = (myAPI *) *log;
  OSBlog *me = (OSBlog *)temp->impl_;
  if (me) delete me;
  me = NULL;
  if (temp) delete temp;
  temp = NULL;
  *log = NULL;

  return VXIlog_RESULT_SUCCESS;
}

