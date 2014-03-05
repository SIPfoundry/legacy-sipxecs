#include <sys/select.h>
#include <sys/time.h>
#include <csignal>

#include "CallManager.h"

static opt::options_description gOptions("Call Generator Options");

template <typename T>
void verify_option(opt::variables_map& vm, const char* option, T& value)
{
  if (!vm.count(option))
  {
    std::cerr << "Please specify value for --" << option << " option" << std::endl;
    std::cout << gOptions << std::endl;
    std::cout << "Example:  ./callgen --cps 10 --max-total 100 --sip-uri 100@domain.com --sip-domain domain.com --sip-user 100 --sip-pass mypass --esl-user callgen --esl-pass somepass --esl-port 6382" << std::endl;
    exit(1);
  }

  value = vm[option].as<T>();
}

CallManager::CallManager(int argc, char** argv) :
  _lastTick(0),
  _duration(0),
  _cps(0),
  _maxCount(0),
  _totalCount(0),
  _terminate(false),
  _eslHost("127.0.0.1"),
  _eslPort(0)
{
 gOptions.add_options()
   ("help", "Display this help message")
   ("cps", opt::value<int>(), "Calls Per Second")
   ("max-total", opt::value<int>(), "Maximum total calls")
   ("sip-uri", opt::value<std::string>(), "Target Uri")
   ("sip-domain", opt::value<std::string>(), "Local domain")
   ("sip-user", opt::value<std::string>(), "SIP user")
   ("sip-pass", opt::value<std::string>(), "SIP password")
   ("esl-user", opt::value<std::string>(), "ESL auth user")
   ("esl-pass", opt::value<std::string>(), "ESL auth password")
   ("esl-port", opt::value<int>(), "ESL port")
   ("bridge-address", opt::value<std::string>(), "WebRTC bridge address")
 ;

  opt::store(opt::parse_command_line(argc, argv, gOptions), _vm);
  opt::notify(_vm);

  if (_vm.count("help"))
  {
    std::cout << gOptions << std::endl;
    exit(1);
  }

  verify_option(_vm, "cps", _cps);
  verify_option(_vm, "max-total", _maxCount);
  verify_option(_vm, "sip-uri", _targetUri);
  verify_option(_vm, "esl-user", _eslUser);
  verify_option(_vm, "esl-pass", _eslPass);
  verify_option(_vm, "esl-port", _eslPort);

  if (_cps > MAX_CPS)
    _cps = MAX_CPS;

  _maxConcurrent = _cps * 2;

  _duration = (1000 / _cps) * 1000;
}

CallManager::~CallManager()
{

}

UInt64 CallManager::getTime()
{
  struct timeval sTimeVal, ret;
	gettimeofday( &sTimeVal, NULL );
  timersub(&sTimeVal, &_bootTime, &ret);
  return (UInt64)( (sTimeVal.tv_sec * 1000000) + sTimeVal.tv_usec );
}

void CallManager::adaptiveWait()
{
  unsigned long nextWait = _duration;
  if (!_lastTick)
  {
    _lastTick = getTime();
  }
  else
  {
    UInt64 now = getTime();
    UInt64 accuracy = now - _lastTick;

    _lastTick = now;
    if (accuracy < _duration)
    {
      //
      // Timer fired too early.  compensate by adding more ticks
      //
      nextWait = _duration + (_duration - accuracy);
      _lastTick = _lastTick + (_duration - accuracy);
    }
    else if (accuracy > _duration)
    {
      //
      // Timer fired too late.  compensate by removing some ticks
      //
      if ((accuracy - _duration) < _duration)
      {
        nextWait = _duration - (accuracy - _duration);
        _lastTick = _lastTick - (accuracy - _duration);
      }
      else
      {
        //
        // We are late by more than the duration value.
        // remove the entire duration value + the delta
        //
        nextWait = 0;
        //
        // We will fire now and offset the differce to the next iteration
        //
        _lastTick = _lastTick - ((accuracy - _duration) - _duration);
      }
    }
  }

  if (nextWait)
  {
    timeval sTimeout = { nextWait / 1000000, ( nextWait % 1000000 ) };
    select( 0, 0, 0, 0, &sTimeout );
  }
}

void CallManager::spawn()
{
  mutex_lock lock(_callMapMutex);
  if (_callMap.size() >= _maxConcurrent)
    return;
  ;

  CallThread* pCall = new CallThread(*this, ++_totalCount);
  _callMap[pCall->_id] = pCall;

  pCall->_targetUri;
  pCall->_authUser;
  pCall->_authPass;
  pCall->_fromUser;
  pCall->_domain;

  pCall->run();
}

void CallManager::collect(int id)
{
  mutex_lock lock(_callMapMutex);
  if (_callMap.find(id) != _callMap.end())
  {
    _deletedCalls.push_back(_callMap[id]);
    _callMap.erase(id);
  }
}

void CallManager::garbageCollect()
{
   mutex_lock lock(_callMapMutex);
  for (DeletedCalls::iterator iter = _deletedCalls.begin(); iter != _deletedCalls.end(); iter++)
  {
    delete (*iter);
  }
  _deletedCalls.clear();
}

void CallManager::displayStatistics()
{
  
}

void CallManager::start()
{
  while(_totalCount < _maxCount && !_terminate)
  {
    spawn();
    garbageCollect();
    adaptiveWait();
  }

  garbageCollect();
  
  displayStatistics();

  exit(0);
}

void CallManager::stop()
{
  _terminate = true;
}

void CallManager::waitForTerminationRequest()
{
	sigset_t sset;
	sigemptyset(&sset);
	sigaddset(&sset, SIGINT);
	sigaddset(&sset, SIGQUIT);
	sigaddset(&sset, SIGTERM);
	sigprocmask(SIG_BLOCK, &sset, NULL);
	int sig;
	sigwait(&sset, &sig);
  std::cout << "Termination Signal RECEIVED" << std::endl;
}
