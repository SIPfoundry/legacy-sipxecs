#ifndef CALLMANAGER_H_INCLUDED
#define	CALLMANAGER_H_INCLUDED

#include <map>
#include <vector>
#include <boost/program_options.hpp>
#include "CallThread.h"

namespace opt = boost::program_options;

#define MAX_CPS 100

class CallManager
{
public:

  typedef boost::recursive_mutex mutex;
  typedef boost::lock_guard<mutex> mutex_lock;
  typedef std::map<int, CallThread*> CallMap;
  typedef std::vector<CallThread*> DeletedCalls;

  CallManager(
    int argc,
    char** argv
    );
  ~CallManager();

  UInt64 getTime();

  void adaptiveWait();

  void start();

  void stop();

  void waitForTerminationRequest();

  void spawn();

  void collect(int id);

  void garbageCollect();

  void displayStatistics();

  const std::string& getEslUser() const;

  const std::string& getEslPass() const;

  const std::string& getEslHost() const;

  int getEslPort() const;

  template <typename T>
  bool getOption(const char* option, T& value)
  {
    if (_vm.find(option) == _vm.end())
      return false;
    value = _vm[option].as<T>();
    return true;
  }
protected:
  opt::variables_map _vm;
  struct timeval _bootTime;
  UInt64 _lastTick;
  int _duration;
  int _cps;
  int _maxCount;
  int _totalCount;
  int _maxConcurrent;
  std::string _targetUri;
  std::string _eslUser;
  std::string _eslPass;
  std::string _eslHost;
  int _eslPort;
  bool _terminate;
  CallMap _callMap;
  DeletedCalls _deletedCalls;
  mutex _callMapMutex;
};

//
// Inlines
//

inline const std::string& CallManager::getEslUser() const
{
  return _eslUser;
}

inline const std::string& CallManager::getEslPass() const
{
  return _eslPass;
}

inline const std::string& CallManager::getEslHost() const
{
  return _eslHost;
}

inline int CallManager::getEslPort() const
{
  return _eslPort;
}


#endif	// CALLMANAGER_H_INCLUDED

