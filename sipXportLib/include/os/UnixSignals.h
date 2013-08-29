/*
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

#ifndef UNIXSIGNALS_H
#define	UNIXSIGNALS_H

#include <signal.h>
#include <pthread.h>
#include <setjmp.h>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace Os
{
  namespace detail
  {
    typedef boost::function<void()> Handler;
    typedef std::vector<Handler> HandlerList;
    typedef std::map<int, HandlerList> HandlerMap;
    typedef boost::lock_guard<boost::recursive_mutex> mutex_lock;
    static HandlerMap _handlers;
    static boost::recursive_mutex _handlersMutex;


    //
    // The jump vector for each thread
    //
    struct JumpBuffer{sigjmp_buf buf;};
    typedef std::vector<JumpBuffer> JumpBufferVec;
    typedef std::map<pthread_t, JumpBufferVec> JumpBuffers;
    static JumpBuffers _jumpBuffers;
    static boost::recursive_mutex _jumpBufferMutex;

    static sigjmp_buf& _pushJumpBuffer()
    {
      mutex_lock lock(_jumpBufferMutex);
      pthread_t tid = ::pthread_self();
      JumpBuffers::iterator jumpBuffer = _jumpBuffers.find(tid);
      if (jumpBuffer == _jumpBuffers.end())
      {
        JumpBufferVec jumpVec;
        _jumpBuffers[tid] = jumpVec;
        JumpBuffer buf;
        _jumpBuffers[tid].push_back(buf);
        return _jumpBuffers[tid].back().buf;
      }
      else
      {
        JumpBuffer buf;
        jumpBuffer->second.push_back(buf);
        return jumpBuffer->second.back().buf;
      }
    }

    static void _popJumpBuffer()
    {
      _jumpBufferMutex.lock();
      pthread_t tid = ::pthread_self();
      JumpBuffers::iterator jumpBuffer = _jumpBuffers.find(tid);
      if (jumpBuffer != _jumpBuffers.end())
      {
        jumpBuffer->second.pop_back();
      }
      _jumpBufferMutex.unlock();
    }

    static sigjmp_buf* _getJumpBuffer()
    {
      mutex_lock lock(_jumpBufferMutex);
      pthread_t tid = ::pthread_self();
      JumpBuffers::iterator jumpBuffer = _jumpBuffers.find(tid);
      if (jumpBuffer != _jumpBuffers.end() && !jumpBuffer->second.empty())
        return &(jumpBuffer->second.back().buf);
      return 0;
    }

    static bool& _is_termination_signal_received()
    {
      static bool received = false;
      return received;
    }

    static void _handle_signal(int sig)
    {
      if (sig == 	SIGINT ||
          sig == 	SIGQUIT ||
          sig == 	SIGABRT ||
          sig == 	SIGKILL ||
          sig == 	SIGTERM )
      {
        bool& termination_signal_received = _is_termination_signal_received();
        termination_signal_received = true;
      }

      if (sig == SIGILL ||
        sig == SIGBUS ||
        sig == SIGSEGV ||
        sig == SIGSYS)
      {
        _jumpBufferMutex.lock();
        //
        // Check if a jump buffer is registered
        //
        sigjmp_buf* pJumpBuff = detail::_getJumpBuffer();
        if (pJumpBuff)
        {
          //
          // Jump to the calling thread if a jump buffer is set
          //
          //
          // Make sure we unlock the mutex before we jump
          //
          _jumpBufferMutex.unlock();
          ::siglongjmp(*pJumpBuff, sig);
        }
        _jumpBufferMutex.unlock();
      }

      _handlersMutex.lock();
      HandlerMap::iterator callBacks = _handlers.find(sig);
      if (callBacks != _handlers.end())
      {
        for (HandlerList::iterator iter = callBacks->second.begin(); iter != callBacks->second.end(); iter++)
        {
          Handler& handler = *iter;
          handler();
        }
      }
      _handlersMutex.unlock();

      if (sig == SIGILL ||
        sig == SIGBUS ||
        sig == SIGSEGV ||
        sig == SIGSYS)
        std::abort();
    }

    static void _register_signal_handler(int sig)
    {
      struct sigaction sa;
      sa.sa_handler = detail::_handle_signal;
      sa.sa_flags   = 0;
      ::sigemptyset(&sa.sa_mask);
      ::sigaction(sig,  &sa, 0);
    }
  }

  class UnixSignals
  {
  public:
    typedef detail::Handler Handler;

    static UnixSignals& instance()
    {
      static UnixSignals me;
      static bool initialized = false;
      if (!initialized)
      {
        detail::_register_signal_handler(SIGHUP);
        detail::_register_signal_handler(SIGINT);
        detail::_register_signal_handler(SIGQUIT);
        detail::_register_signal_handler(SIGILL);
        detail::_register_signal_handler(SIGTRAP);
        detail::_register_signal_handler(SIGABRT);
        detail::_register_signal_handler(SIGBUS);
        detail::_register_signal_handler(SIGFPE);
        detail::_register_signal_handler(SIGKILL);
        detail::_register_signal_handler(SIGUSR1);
        detail::_register_signal_handler(SIGSEGV);
        detail::_register_signal_handler(SIGUSR2);
        detail::_register_signal_handler(SIGPIPE);
        detail::_register_signal_handler(SIGALRM);
        detail::_register_signal_handler(SIGTERM);
        detail::_register_signal_handler(SIGSTKFLT);
        detail::_register_signal_handler(SIGCHLD);
        detail::_register_signal_handler(SIGCONT);
        detail::_register_signal_handler(SIGSTOP);
        detail::_register_signal_handler(SIGTSTP);
        detail::_register_signal_handler(SIGTTIN);
        detail::_register_signal_handler(SIGTTOU);
        detail::_register_signal_handler(SIGURG);
        detail::_register_signal_handler(SIGXCPU);
        detail::_register_signal_handler(SIGXFSZ);
        detail::_register_signal_handler(SIGVTALRM);
        detail::_register_signal_handler(SIGPROF);
        detail::_register_signal_handler(SIGWINCH);
        detail::_register_signal_handler(SIGPOLL);
        detail::_register_signal_handler(SIGPWR);
        detail::_register_signal_handler(SIGSYS);
        initialized = true;
      }
      return me;
    };

    bool& isTerminateSignalReceived()
    {
      return detail::_is_termination_signal_received();
    }

    void registerSignalHandler(int sig, Handler handler)
    {
      detail::_handlersMutex.lock();
      detail::HandlerMap::iterator handlers = detail::_handlers.find(sig);
      if (handlers == detail::_handlers.end())
      {
        detail::HandlerList handlerList;
        handlerList.push_back(handler);
        detail::_handlers[sig] = handlerList;
      }
      else
      {
        handlers->second.push_back(handler);
      }
      detail::_handlersMutex.unlock();
    }

  private:
    UnixSignals(){}
    UnixSignals(const UnixSignals&){}
  };

  class UnixSignalTrap : public std::exception
  {
  public:
    UnixSignalTrap() :
      _jumpBuffer(0),
      _sig(0)
    {
    }

    UnixSignalTrap(int sig) :
      _jumpBuffer(0),
      _sig(sig)
    {
    }

    ~UnixSignalTrap() throw()
    {
      if (_jumpBuffer)
        Os::detail::_popJumpBuffer();
    }

    void throwSignalException(int sig)
    {
      if (sig)
        throw UnixSignalTrap(sig);
    }

    const char* what() const throw()
    {
      switch (_sig)
      {
        case	SIGHUP:
          return "Hangup (POSIX)";
        case	SIGINT:
          return "Interrupt (ANSI)";
        case	SIGQUIT:
          return "Quit (POSIX)";
        case	SIGILL:
          return "Illegal instruction (ANSI)";
        case	SIGTRAP:
          return "Trace trap (POSIX)";
        case	SIGABRT:
          return "Abort (ANSI)";
        case	SIGBUS:
          return "BUS error (4.2 BSD)";
        case	SIGFPE:
          return "Floating-point exception (ANSI)";
        case	SIGKILL:
          return "Kill, unblockable (POSIX)";
        case	SIGUSR1:
          return "User-defined signal 1 (POSIX)";
        case	SIGSEGV:
          return "Segmentation violation (ANSI)";
        case	SIGUSR2:
          return "User-defined signal 2 (POSIX)";
        case	SIGPIPE:
          return "Broken pipe (POSIX)";
        case	SIGALRM:
          return "Alarm clock (POSIX)";
        case	SIGTERM:
          return "Termination (ANSI)";
        case	SIGSTKFLT:
          return "Stack fault";
        case	SIGCHLD:
          return "Child status has changed (POSIX)";
        case	SIGCONT:
          return "Continue (POSIX)";
        case	SIGSTOP:
          return "Stop, unblockable (POSIX)";
        case	SIGTSTP:
          return "Keyboard stop (POSIX)";
        case	SIGTTIN:
          return "Background read from tty (POSIX)";
        case	SIGTTOU:
          return "Background write to tty (POSIX)";
        case	SIGURG:
          return "Urgent condition on socket (4.2 BSD)";
        case	SIGXCPU:
          return "CPU limit exceeded (4.2 BSD)";
        case	SIGXFSZ:
          return "File size limit exceeded (4.2 BSD)";
        case	SIGVTALRM:
          return "Virtual alarm clock (4.2 BSD)";
        case	SIGPROF:
          return "Profiling alarm clock (4.2 BSD)";
        case	SIGWINCH:
          return "Window size change (4.3 BSD, Sun)";
        case	SIGPOLL:
          return "Pollable event occurred (System V)";
        case	SIGPWR:
          return "Power failure restart (System V)";
        case SIGSYS:
          return "Bad system call";
        default:
          return "Unknown Signal Caught";
      }
    }

    int sig() const{ return _sig; }

    sigjmp_buf* jumpBuffer()
    {
      if (!_jumpBuffer)
        _jumpBuffer = &(Os::detail::_pushJumpBuffer());
      return _jumpBuffer;
    }
  private:
    sigjmp_buf* _jumpBuffer;
    int _sig;
  };

  #define TrapUnixSignals Os::UnixSignalTrap __os_signal_trap__; \
  __os_signal_trap__.throwSignalException(::sigsetjmp(*(__os_signal_trap__.jumpBuffer()), 1));


}


#endif	/* UNIXSIGNALS_H */

