/*
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */

#ifndef OSEXCEPTIONHANDLER_H_
#define OSEXCEPTIONHANDLER_H_

#include <os/OsLogger.h>
#include <os/OsStatus.h>
#include <map>
#include <vector>

#define STD_GENERAL_EXCEPTION           "stdException"
#define BOOST_GENERAL_EXCEPTION         "boostException"
#define MONGO_SOCKET_EXCEPTION          "mongoSocketException"
#define MONGO_CONNECT_EXCEPTION         "mongoConnectException"
#define MONGO_GENERAL_EXCEPTION         "mongoDBException"
#define UNKNOWN_EXCEPTION_MESSAGE       "Error occurred. Unknown exception type."

//exception handling method types
typedef boost::function<void (std::exception& e)> ExceptionHandler;
typedef boost::function<void (boost::exception& e)> BoostExceptionHandler;

//vectors in which exception handling methods are stored
typedef std::vector<ExceptionHandler> HandlerVector;
typedef std::vector<BoostExceptionHandler> BoostHandlerVector;

//containers with entries like < exceptionName, handlersVector>
typedef std::map< const std::string, HandlerVector > HandlerMap;
typedef std::map<const std::string, BoostHandlerVector > BoostHandlerMap;

enum ExceptionType { STD_EXCEPTION, MONGO_EXCEPTION };

#define catch_global_print(msg)  \
  std::ostringstream bt; \
  bt << msg << std::endl; \
  void* trace_elems[20]; \
  int trace_elem_count(backtrace( trace_elems, 20 )); \
  char** stack_syms(backtrace_symbols(trace_elems, trace_elem_count)); \
  for (int i = 0 ; i < trace_elem_count ; ++i ) \
    bt << stack_syms[i] << std::endl; \
  Os::Logger::instance().log(FAC_LOG, PRI_CRIT, bt.str().c_str()); \
  std::cerr << bt.str().c_str(); \
  free(stack_syms);

//
// Class for exception handling
//
class OsExceptionHandler
{
private:
  // constructor
  OsExceptionHandler()
  {
  }

  // copy constructor
  OsExceptionHandler(OsExceptionHandler const&){};

  ~OsExceptionHandler();

  // returns specific handlers vector corresponding to key
  template<typename TContainer, typename THandlers>
  inline static OsStatus getHandlers(const std::string& key, TContainer& handlersMap, THandlers& handlerVector);

  //insert a pair of < string , ExceptionHandler > in specified HandlerMap
  inline static OsStatus insertInContainer(const std::string& key, ExceptionHandler f, HandlerMap& exceptionContainer);

  //calls all registered handlers from HandlerVector
  static void callHandlers(HandlerVector handlers, std::exception& e);

  //calls all registered handlers from BoostHandlerVector
  static void callBoostHandlers(BoostHandlerVector handlers, boost::exception& e);

  static OsStatus treatException(ExceptionType eType, const std::string& key, HandlerMap& handlerMap, std::exception& e);
  static OsStatus treatBoostException(const std::string& key, BoostHandlerMap& handlerMap, boost::exception& e);

  //friend class for testing
  friend class OsExceptionHandlerTest;
public:
  //singleton instance
  inline static OsExceptionHandler& instance();

  inline static bool isInstantiated();
  inline static bool isTerminated();

  // clear all containers with exception handlers
  inline static void clearHandlers();

  // default exception handlers
  inline void defaultStdGeneralExceptionHandler(std::exception& e);
  inline void defaultBoostGeneralExceptionHandler(boost::exception& e);
#ifdef MONGO_wassert
  inline void defaultMongoSocketExceptionHandling(std::exception& e);
  inline void defaultMongoConnectExceptionHandling(std::exception& e);
  inline void defaultMongoGeneralExceptionHandler(std::exception& e);
#endif

  // registering exception handlers
  OsStatus registerHandler(ExceptionType type, const std::string& key, ExceptionHandler f);
  OsStatus registerHandler(const std::string& key, BoostExceptionHandler f);

  //
  //registers the default exception handling methods
  // std general, boost general, mongo socket, mongo connect, mongo general exceptions
  OsStatus registerDefaultHandlers();

  //catch exceptions and call corresponding registered exception handler
  static void catch_global();

private:
  static bool _instantiatedFlag;
  static bool _terminatedFlag;

  // exception containers
  static HandlerMap _stdHandlersContainer;
  static BoostHandlerMap _boostHandlersContainer;
  static HandlerMap _mongoHandlersContainer;
};

OsExceptionHandler::~OsExceptionHandler()
{
  if(false == isTerminated())
  {
    clearHandlers();
    _terminatedFlag = true;
  }
}

OsExceptionHandler&  OsExceptionHandler::instance()
{
    static OsExceptionHandler singleton;
    singleton.registerDefaultHandlers();
    _instantiatedFlag = true;

    return singleton;
}

bool OsExceptionHandler::isInstantiated()
{
  return _instantiatedFlag;
}

bool OsExceptionHandler::isTerminated()
{
  return _terminatedFlag;
}

void OsExceptionHandler::clearHandlers()
{
  // clear exception handlers containers
  _stdHandlersContainer.clear();
  _boostHandlersContainer.clear();
  _mongoHandlersContainer.clear();
}

//default general std exception handling : log & abort
void OsExceptionHandler::defaultStdGeneralExceptionHandler(std::exception& e)
{
  catch_global_print(e.what());
  std::abort();
}

//default general boost exception handling : log & abort
void OsExceptionHandler::defaultBoostGeneralExceptionHandler(boost::exception& e)
{
  catch_global_print(diagnostic_information(e).c_str());
  std::abort();
}

#ifdef MONGO_wassert
//default mongo socket exception handling : log & exit
void OsExceptionHandler::defaultMongoSocketExceptionHandling(std::exception& e)
{
  catch_global_print(static_cast<mongo::DBException&>(e).toString().c_str());
  _exit(1);
}

//default mongo connect exception handling : log & exit
void OsExceptionHandler::defaultMongoConnectExceptionHandling(std::exception& e)
{
  catch_global_print(static_cast<mongo::DBException&>(e).toString().c_str());
  _exit(1);
}

//default mongo general exception handling : log & abort
void OsExceptionHandler::defaultMongoGeneralExceptionHandler(std::exception& e)
{
  catch_global_print(static_cast<mongo::DBException&>(e).toString().c_str());
  std::abort();
}
#endif

template <typename Container, typename ExceptionHandlers>
OsStatus OsExceptionHandler::getHandlers(const std::string& key, Container& handlersMap, ExceptionHandlers& handlerVector)
{
  //search in container for vector containing the handlers by key
  if(handlersMap.end() != handlersMap.find(key))
  {
    handlerVector = handlersMap.at(key);
    return OS_SUCCESS;
  }
  // vector not found
  return OS_FAILED;
}

OsStatus OsExceptionHandler::insertInContainer(const std::string& key, ExceptionHandler f, HandlerMap& exceptionContainer)
{
  if(exceptionContainer.end() == exceptionContainer.find(key))
  {
    // key not found, insert new pair
    HandlerVector handlers;
    handlers.push_back(f);
    exceptionContainer.insert(std::pair<const std::string, HandlerVector>(key, handlers));

    return OS_SUCCESS;
  }
  else
  {
    const HandlerVector::iterator it = exceptionContainer.at(key).begin();

    // key already in map -> insert as first element in vector
    exceptionContainer.at(key).insert(it, f);
    return OS_SUCCESS;
  }
}

// register a ExceptionHandler in corresponding container
OsStatus OsExceptionHandler::registerHandler(ExceptionType type, const std::string& key, ExceptionHandler f)
{
  switch(type)
  {
    case STD_EXCEPTION :
      insertInContainer(key, f, _stdHandlersContainer);
      break;
    case MONGO_EXCEPTION :
      insertInContainer(key, f, _mongoHandlersContainer);
      break;
    default:
      return OS_FAILED;
  }
  return OS_SUCCESS;
}

// register a boost exception handler
OsStatus OsExceptionHandler::registerHandler(const std::string& key, BoostExceptionHandler f)
{
  if(_boostHandlersContainer.end() == _boostHandlersContainer.find(key))
  {
    // key not found, insert new pair
    BoostHandlerVector handlers;
    handlers.push_back(f);
    _boostHandlersContainer.insert(std::pair<const std::string, BoostHandlerVector>(key, handlers));

    return OS_SUCCESS;
  }
  else
  {
    const BoostHandlerVector::iterator it = _boostHandlersContainer.at(key).begin();
    // key already in map -> push back in vector
    _boostHandlersContainer.at(key).insert(it, f);

    return OS_SUCCESS;
  }
}

OsStatus OsExceptionHandler::registerDefaultHandlers()
{
  OsStatus returnStatus = OS_SUCCESS;

  if(false == _instantiatedFlag)
  {
    //register default std handler
    ExceptionHandler defaultStdHandler = static_cast<ExceptionHandler>(boost::bind(&OsExceptionHandler::defaultStdGeneralExceptionHandler, this, _1));
    if(returnStatus != registerHandler(STD_EXCEPTION, STD_GENERAL_EXCEPTION, defaultStdHandler))
    {
      return OS_FAILED;
    }

    //register default boost handler
    BoostExceptionHandler defaultBoostHandler = static_cast<BoostExceptionHandler>(boost::bind(&OsExceptionHandler::defaultBoostGeneralExceptionHandler, this, _1));
    if(returnStatus != registerHandler(BOOST_GENERAL_EXCEPTION, defaultBoostHandler))
    {
      return OS_FAILED;
    }

#ifdef MONGO_wassert
    // register socket exception handler
    ExceptionHandler  defaultMongoHandler = static_cast<ExceptionHandler>(boost::bind(&OsExceptionHandler::defaultMongoSocketExceptionHandling, this, _1));
    if(returnStatus != registerHandler(MONGO_EXCEPTION, MONGO_SOCKET_EXCEPTION, defaultMongoHandler))
    {
      return OS_FAILED;
    }

    // register connect exception handler
    defaultMongoHandler = static_cast<ExceptionHandler>(boost::bind(&OsExceptionHandler::defaultMongoConnectExceptionHandling, this, _1));
    if(returnStatus != registerHandler(MONGO_EXCEPTION, MONGO_CONNECT_EXCEPTION, defaultMongoHandler))
    {
      return OS_FAILED;
    }

    // register general exception handler
    defaultMongoHandler = static_cast<ExceptionHandler>(boost::bind(&OsExceptionHandler::defaultMongoGeneralExceptionHandler, this, _1));
    if(returnStatus != registerHandler(MONGO_EXCEPTION, MONGO_GENERAL_EXCEPTION, defaultMongoHandler))
    {
      return OS_FAILED;
    }
#endif
    return OS_SUCCESS;
  }
  else
  {
    //another instance is already running
    return OS_FAILED;
  }
}

void OsExceptionHandler::callHandlers(HandlerVector handlers, std::exception& e)
{
  for(size_t i = 0; i < handlers.size(); ++i)
  {
    handlers.at(i)(e);
  }
}

void OsExceptionHandler::callBoostHandlers(BoostHandlerVector handlers, boost::exception& e)
{
  for(size_t i = 0; i < handlers.size(); ++i)
  {
    handlers.at(i)(e);
  }
}

OsStatus OsExceptionHandler::treatException(ExceptionType eType, const std::string& key, HandlerMap& handlerMap, std::exception& e)
{
  //search for specific exception handlers
  OsStatus returnStatus = OS_FAILED;
  HandlerVector handlers;
  if(OS_SUCCESS == getHandlers(key, handlerMap, handlers))
  {
    //handlers found
    // call all registered handlers in vector
    callHandlers(handlers, e);
    return OS_SUCCESS;
  }
  else
  {
    // specific handlers not found, try to call general ones
    switch(eType)
    {
      case STD_EXCEPTION :
        if(key.compare(STD_GENERAL_EXCEPTION) == 0)
        {
          //general std handlers not found
          return OS_FAILED;
        }
        //call general std handlers
        returnStatus = treatException(STD_EXCEPTION, STD_GENERAL_EXCEPTION, _stdHandlersContainer, e);
        break;

      case MONGO_EXCEPTION :
        if(key.compare(MONGO_GENERAL_EXCEPTION) == 0)
        {
          //general mongo handlers not found
          return OS_FAILED;
        }
        //call general mongo handlers
        returnStatus = treatException(MONGO_EXCEPTION, MONGO_GENERAL_EXCEPTION, _mongoHandlersContainer, e);
        break;
      default :
        return OS_FAILED;
    }

    return returnStatus;
  }
}

OsStatus OsExceptionHandler::treatBoostException(const std::string& key, BoostHandlerMap& handlerMap, boost::exception& e)
{
  BoostHandlerVector handlers;
  //search for specific exception handlers
  if(OS_SUCCESS == getHandlers(key, handlerMap, handlers))
  {
    //handlers found
    // call specific registered handlers in vector
    callBoostHandlers(handlers, e);
    return OS_SUCCESS;
  }
  else
  {
    BoostHandlerVector boostDefaultHandlers;

    //general handler not found
    if(key.compare(BOOST_GENERAL_EXCEPTION) == 0)
    {
      //boost general handler not found
      return OS_FAILED;
    }

    //general handler found
    return treatBoostException(BOOST_GENERAL_EXCEPTION, _boostHandlersContainer, e);
  }
}

//catch exceptions and call handler methods from containers
void OsExceptionHandler::catch_global()
{
  OsStatus returnStatus = OS_SUCCESS;
  try
  {
    throw;
  }
#ifdef MONGO_wassert
  catch(mongo::SocketException &e)
  {
    HandlerVector handlers;
    returnStatus = treatException(MONGO_EXCEPTION, MONGO_SOCKET_EXCEPTION, _mongoHandlersContainer, e);
    if(OS_FAILED == returnStatus)
    {
      // no mongo related handler found, call std_general one
      returnStatus = treatException(STD_EXCEPTION, STD_GENERAL_EXCEPTION, _stdHandlersContainer, e);
    }
  }

  catch(mongo::ConnectException &e)
  {
    // try to call mongo connect exception handler
    HandlerVector handlers;
    returnStatus = treatException(MONGO_EXCEPTION, MONGO_CONNECT_EXCEPTION, _mongoHandlersContainer, e);
    if(OS_FAILED == returnStatus)
    {
      // no mongo related handler found, call std_general one
      returnStatus = treatException(STD_EXCEPTION, STD_GENERAL_EXCEPTION, _stdHandlersContainer, e);
    }
  }

  catch(mongo::DBException &e)
  {
    // try to call general mongo exception handler
    HandlerVector handlers;
    returnStatus = treatException(MONGO_EXCEPTION, MONGO_GENERAL_EXCEPTION, _mongoHandlersContainer, e);
    if(OS_FAILED == returnStatus)
    {
      // no mongo related handler found, call std_general one
      returnStatus = treatException(STD_EXCEPTION, STD_GENERAL_EXCEPTION, _stdHandlersContainer, e);
    }
  }
#endif

  catch(boost::exception& e)
  {
    BoostHandlerVector handlers;
    returnStatus = treatBoostException(BOOST_GENERAL_EXCEPTION, _boostHandlersContainer, e);
  }

  catch(std::exception& e)
  {
    HandlerVector handlers;
    returnStatus = treatException(STD_EXCEPTION, STD_GENERAL_EXCEPTION, _stdHandlersContainer, e);
  }

  catch(...)
  {
    catch_global_print(UNKNOWN_EXCEPTION_MESSAGE);
    std::abort();
  }

  //exception was caught but no handlers are registered
  if(OS_FAILED == returnStatus)
  {
    std::abort();
  }
}

#endif /* OSEXCEPTIONHANDLER_H_ */
