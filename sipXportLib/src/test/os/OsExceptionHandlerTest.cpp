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

// OsExceptionHandlerTest class needs to access OsExceptionHandler private members
// and variables. In order to test OsExceptionHandler functionality OsExceptionHandlerTest
// should be a friend class of OsExceptionHandler

#include <iostream>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <mongo/util/assert_util.h>
#include <mongo/client/dbclient.h>
#include <mongo/util/net/sock.h>
#include <os/OsExceptionHandler.h>

#define NO_ACTION_TAKEN   0
#define EXCEPTION_HANDLED 1
#define PHONEY_KEY        "phony"
#define CUSTOM_STD_HANDLER    "custom_std"
#define CUSTOM_BOOST_HANDLER    "custom_boost"
#define CUSTOM_MONGO_SOCKET_HANDLER    "custom_mongo_socket"
#define CUSTOM_MONGO_CONNECTT_HANDLER    "custom_mongo_connect"
#define CUSTOM_MONGO_HANDLER    "custom_mongo_general"


//custom boost exception
struct boost_exception: virtual boost::exception, virtual std::exception { };

// variables used to see if treating exception method has been called
bool stdAction = NO_ACTION_TAKEN;
bool secondStdAction = NO_ACTION_TAKEN;
bool boostAction = NO_ACTION_TAKEN;
bool secondBoostAction = NO_ACTION_TAKEN;
bool mongoConnectAction = NO_ACTION_TAKEN;
bool secondMongoConnectAction = NO_ACTION_TAKEN;
bool mongoSocketAction = NO_ACTION_TAKEN;
bool secondMongoSocketAction = NO_ACTION_TAKEN;
bool mongoAction = NO_ACTION_TAKEN;
bool secondMongoAction = NO_ACTION_TAKEN;

#define NO_REGISTERED_HANDLERS    0

class OsExceptionHandlerTest : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(OsExceptionHandlerTest);
  CPPUNIT_TEST(singleInstanceTest);
  CPPUNIT_TEST(customStdHandlerTest);
  CPPUNIT_TEST(customBoostHandlerTest);
  CPPUNIT_TEST(customMongoHandlerTest);
  CPPUNIT_TEST(checkNoSpecificHandlers);
  CPPUNIT_TEST(getHandlersTest);
  CPPUNIT_TEST(insertInContainerTest);
  CPPUNIT_TEST_SUITE_END();

public:

//
// methods used by tests
//
  void reInitTestVariables()
  {
    stdAction = NO_ACTION_TAKEN;
    secondStdAction = NO_ACTION_TAKEN;
    boostAction = NO_ACTION_TAKEN;
    secondBoostAction = NO_ACTION_TAKEN;
    mongoConnectAction = NO_ACTION_TAKEN;
    secondMongoConnectAction = NO_ACTION_TAKEN;
    mongoSocketAction = NO_ACTION_TAKEN;
    secondMongoSocketAction = NO_ACTION_TAKEN;
    mongoAction = NO_ACTION_TAKEN;
    secondMongoAction = NO_ACTION_TAKEN;
  }

  //check if size of containers correspond
  OsStatus verifyContainersSize(size_t numStdEntries, size_t numBoostHandlersEntries, size_t numMongoHandlersEntries)
  {
    if( numStdEntries != OsExceptionHandler::_stdHandlersContainer.size() ||
        numBoostHandlersEntries != OsExceptionHandler::_boostHandlersContainer.size() ||
        numMongoHandlersEntries != OsExceptionHandler::_mongoHandlersContainer.size())
    {
      return OS_FAILED;
    }
    return OS_SUCCESS;
  }

  //check if correct number of handlers are registered
  OsStatus verifyNumberOfHandlers(HandlerVector handlers, size_t expectedNumber)
  {
    if(handlers.size() == expectedNumber)
    {
      return OS_SUCCESS;
    }
    return OS_FAILED;
  }

  //check if correct number of boost handlers are registered
  OsStatus verifyNumberOfBoostHandlers(BoostHandlerVector handlers, size_t expectedNumber)
  {
    if(handlers.size() == expectedNumber)
    {
      return OS_SUCCESS;
    }
    return OS_FAILED;
  }

  //
  // custom exception handlers
  //
  void phonyStdExceptionHandler(std::exception& e)
  {
    stdAction = EXCEPTION_HANDLED;

  }

  void phonyBoostExceptionHandler(boost::exception& e)
  {
    boostAction = EXCEPTION_HANDLED;
  }

  void phonyMongoExceptionHandler(std::exception& e)
  {
    mongoAction =  EXCEPTION_HANDLED;
  }

  void phonyMongoSocketExceptionHandler(std::exception& e)
  {
    mongoSocketAction = EXCEPTION_HANDLED;
  }

  void phonyMongoConnectExceptionHandler(std::exception& e)
  {
    mongoConnectAction = EXCEPTION_HANDLED;
  }
  void secondStdExceptionHandler(std::exception& e)
  {
    secondStdAction = EXCEPTION_HANDLED;

  }
  void secondBoostExceptionHandler(boost::exception& e)
  {
    secondBoostAction = EXCEPTION_HANDLED;
  }

  void secondMongoExceptionHandler(std::exception& e)
  {
    secondMongoAction =  EXCEPTION_HANDLED;
  }

  void secondMongoSocketExceptionHandler(std::exception& e)
  {
    secondMongoSocketAction = EXCEPTION_HANDLED;
  }

  void secondMongoConnectExceptionHandler(std::exception& e)
  {
    secondMongoConnectAction = EXCEPTION_HANDLED;
  }

//
// test methods
//
  void singleInstanceTest()
  {
    //TEST:: test if object is not instantiated
    CPPUNIT_ASSERT(false == OsExceptionHandler::isInstantiated());

    OsExceptionHandler::instance();

    //TEST: check if object was correctly instantiated
    CPPUNIT_ASSERT(true == OsExceptionHandler::isInstantiated());

    //TEST: check if _terminatedFlag is set to false
    CPPUNIT_ASSERT(false == OsExceptionHandler::isTerminated());

    //TEST: try to register default handlers. This should fail
    //because the default handlers can be registered only once
    CPPUNIT_ASSERT(OS_FAILED == OsExceptionHandler::instance().registerDefaultHandlers());

    //
    // Tests that verify that the correct number of handlers are registered
    //
    //TEST: check number of entries in exception containers
    CPPUNIT_ASSERT(OS_SUCCESS == verifyContainersSize(1, 1, 3));
    //TEST: number of std handlers has to be 1
    CPPUNIT_ASSERT(OS_SUCCESS == verifyNumberOfHandlers(OsExceptionHandler::_stdHandlersContainer.at(STD_GENERAL_EXCEPTION), 1));
    //TEST: number of boost handlers has to be 1
    CPPUNIT_ASSERT(OS_SUCCESS == verifyNumberOfBoostHandlers(OsExceptionHandler::_boostHandlersContainer.at(BOOST_GENERAL_EXCEPTION), 1));
    //TEST: number of mongo general handlers has to be 1
    CPPUNIT_ASSERT(OS_SUCCESS == verifyNumberOfHandlers(OsExceptionHandler::_mongoHandlersContainer.at(MONGO_GENERAL_EXCEPTION), 1));
    //TEST: number of mongo socket handlers has to be 1
    CPPUNIT_ASSERT(OS_SUCCESS == verifyNumberOfHandlers(OsExceptionHandler::_mongoHandlersContainer.at(MONGO_SOCKET_EXCEPTION), 1));
    //TEST: number of mongo connect handlers has to be 1
    CPPUNIT_ASSERT(OS_SUCCESS == verifyNumberOfHandlers(OsExceptionHandler::_mongoHandlersContainer.at(MONGO_CONNECT_EXCEPTION), 1));

    // clear all default handlers for next tests
    // this is needed because the default handling methods exit or abort
    OsExceptionHandler::clearHandlers();
    //TEST: check if containers are emtpy
    CPPUNIT_ASSERT(OS_SUCCESS == verifyContainersSize(0, 0, 0));
  }

  void customStdHandlerTest()
  {
    reInitTestVariables();

    //register custom std handler
    ExceptionHandler customStdHandler = static_cast<ExceptionHandler>(boost::bind(&OsExceptionHandlerTest::phonyStdExceptionHandler, this, _1));
    OsExceptionHandler::instance().registerHandler(STD_EXCEPTION, STD_GENERAL_EXCEPTION, customStdHandler);

    try
    {
      throw std::exception();
    }
    catch(std::exception& e)
    {
      OsExceptionHandler::catch_global();
    }

    //TEST : check if custom handler action was taken
    CPPUNIT_ASSERT(EXCEPTION_HANDLED == stdAction);

    //reset test variables
    reInitTestVariables();

    //register a second one
    ExceptionHandler secondCustomStdHandler = static_cast<ExceptionHandler>(boost::bind(&OsExceptionHandlerTest::secondStdExceptionHandler, this, _1));
    OsExceptionHandler::instance().registerHandler(STD_EXCEPTION, STD_GENERAL_EXCEPTION, secondCustomStdHandler);

    //TEST: check number of entries in exception containers
    CPPUNIT_ASSERT(OS_SUCCESS == verifyContainersSize(1, 0, 0));
    //TEST: number of std handlers has to be 2
    CPPUNIT_ASSERT(OS_SUCCESS == verifyNumberOfHandlers(OsExceptionHandler::_stdHandlersContainer.at(STD_GENERAL_EXCEPTION), 2));

    //check if all actions are performed
    try
    {
      throw std::exception();
    }
    catch(std::exception& e)
    {
      OsExceptionHandler::catch_global();
    }

    //TEST : check if first custom handlers action was taken
     CPPUNIT_ASSERT(EXCEPTION_HANDLED == stdAction);
     //TEST : check if second custom handlers action was taken
     CPPUNIT_ASSERT(EXCEPTION_HANDLED == secondStdAction);
  }

  void customBoostHandlerTest()
  {
    reInitTestVariables();

    // register custom boost handler
    BoostExceptionHandler customBoostHandler = static_cast<BoostExceptionHandler>(boost::bind(&OsExceptionHandlerTest::phonyBoostExceptionHandler, this, _1));
    OsExceptionHandler::instance().registerHandler(BOOST_GENERAL_EXCEPTION, customBoostHandler);

    try
    {
      throw boost_exception();
    }
    catch(boost::exception& e)
    {
      OsExceptionHandler::catch_global();
    }

    //TEST : check if custom handler action was taken
    CPPUNIT_ASSERT(EXCEPTION_HANDLED == boostAction);

    //reset test variables
    reInitTestVariables();

    // register second custom boost handler
    BoostExceptionHandler secondCustomBoostHandler = static_cast<BoostExceptionHandler>(boost::bind(&OsExceptionHandlerTest::secondBoostExceptionHandler, this, _1));
    OsExceptionHandler::instance().registerHandler(BOOST_GENERAL_EXCEPTION, secondCustomBoostHandler);

    //TEST: check number of entries in exception containers
    CPPUNIT_ASSERT(OS_SUCCESS == verifyContainersSize(1, 1, 0));
    //TEST: number of std handlers has to be 2
    CPPUNIT_ASSERT(OS_SUCCESS == verifyNumberOfBoostHandlers(OsExceptionHandler::_boostHandlersContainer.at(BOOST_GENERAL_EXCEPTION), 2));

    try
    {
      throw boost_exception();
    }
    catch(boost::exception& e)
    {
      OsExceptionHandler::catch_global();
    }

    //TEST : check if first custom handler action was taken
    CPPUNIT_ASSERT(EXCEPTION_HANDLED == boostAction);
    //TEST : check if second custom handler action was taken
    CPPUNIT_ASSERT(EXCEPTION_HANDLED == secondBoostAction);
  }

   void customMongoHandlerTest()
   {
     reInitTestVariables();

     // register custom mongo socket handler
     ExceptionHandler customMongoSocketHandler = static_cast<ExceptionHandler>(boost::bind(&OsExceptionHandlerTest::phonyMongoSocketExceptionHandler, this, _1));
     OsExceptionHandler::instance().registerHandler(MONGO_EXCEPTION, MONGO_SOCKET_EXCEPTION, customMongoSocketHandler);

     // register custom mongo connect handler
     ExceptionHandler customMongoConnectHandler = static_cast<ExceptionHandler>(boost::bind(&OsExceptionHandlerTest::phonyMongoConnectExceptionHandler, this, _1));
     OsExceptionHandler::instance().registerHandler(MONGO_EXCEPTION, MONGO_CONNECT_EXCEPTION, customMongoConnectHandler);

     // register custom mongo general handler
     ExceptionHandler customMongoHandler = static_cast<ExceptionHandler>(boost::bind(&OsExceptionHandlerTest::phonyMongoExceptionHandler, this, _1));
     OsExceptionHandler::instance().registerHandler(MONGO_EXCEPTION, MONGO_GENERAL_EXCEPTION, customMongoHandler);

     //TEST: check number of entries in exception containers
     CPPUNIT_ASSERT(OS_SUCCESS == verifyContainersSize(1, 1, 3));

     //
     // throw mongo socket exception and check if action was taken
     //
     try
     {
       throw mongo::SocketException(mongo::SocketException::SEND_ERROR, "");
     }
     catch(mongo::SocketException& e)
     {
       OsExceptionHandler::catch_global();
     }
     //TEST : check if new custom socket connect defined handler is executed
     CPPUNIT_ASSERT(EXCEPTION_HANDLED == mongoSocketAction);

     //
     // throw mongo connect exception and check if action was taken
     //
     try
     {
       throw mongo::ConnectException("");
     }
     catch(mongo::ConnectException& e)
     {
       OsExceptionHandler::catch_global();
     }
     //TEST : check if new custom mongo connect defined handler is executed
     CPPUNIT_ASSERT(EXCEPTION_HANDLED == mongoConnectAction);

     //
     try
     {
       throw mongo::DBException("", 0);
     }
     catch(mongo::DBException& e)
     {
       OsExceptionHandler::catch_global();
     }
     //TEST : check if new custom mongo connect defined handler is executed
     //
     CPPUNIT_ASSERT(EXCEPTION_HANDLED == mongoAction);

     reInitTestVariables();

     // register second custom mongo socket handler
     ExceptionHandler secondCustomMongoSocketHandler = static_cast<ExceptionHandler>(boost::bind(&OsExceptionHandlerTest::secondMongoSocketExceptionHandler, this, _1));
     OsExceptionHandler::instance().registerHandler(MONGO_EXCEPTION, MONGO_SOCKET_EXCEPTION, secondCustomMongoSocketHandler);

     // register second custom mongo connect handler
     ExceptionHandler secondCustomMongoConnectHandler = static_cast<ExceptionHandler>(boost::bind(&OsExceptionHandlerTest::secondMongoConnectExceptionHandler, this, _1));
     OsExceptionHandler::instance().registerHandler(MONGO_EXCEPTION, MONGO_CONNECT_EXCEPTION, secondCustomMongoConnectHandler);

     // register second custom mongo general handler
     ExceptionHandler secondCustomMongoHandler = static_cast<ExceptionHandler>(boost::bind(&OsExceptionHandlerTest::secondMongoExceptionHandler, this, _1));
     OsExceptionHandler::instance().registerHandler(MONGO_EXCEPTION, MONGO_GENERAL_EXCEPTION, secondCustomMongoHandler);

     //TEST: check number of entries in exception containers
     CPPUNIT_ASSERT(OS_SUCCESS == verifyContainersSize(1, 1, 3));
     //TEST: number of mongo socket handlers has to be 2
     CPPUNIT_ASSERT(OS_SUCCESS == verifyNumberOfHandlers(OsExceptionHandler::_mongoHandlersContainer.at(MONGO_SOCKET_EXCEPTION), 2));
     //TEST: number of mongo connect handlers has to be 2
     CPPUNIT_ASSERT(OS_SUCCESS == verifyNumberOfHandlers(OsExceptionHandler::_mongoHandlersContainer.at(MONGO_CONNECT_EXCEPTION), 2));
     //TEST: number of mongo general handlers has to be 2
     CPPUNIT_ASSERT(OS_SUCCESS == verifyNumberOfHandlers(OsExceptionHandler::_mongoHandlersContainer.at(MONGO_GENERAL_EXCEPTION), 2));

     //
     // throw socket exception
     // there are 2 registered custom socket handlers
     // check if specific socket registered handlers are called
     //
     try
     {
       throw mongo::SocketException(mongo::SocketException::SEND_ERROR, "");
     }
     catch(mongo::SocketException& e)
     {
       OsExceptionHandler::catch_global();
     }
     //TEST : check if first socket action taken
     CPPUNIT_ASSERT(EXCEPTION_HANDLED == mongoSocketAction);
     //TEST : check if second socket action taken
     CPPUNIT_ASSERT(EXCEPTION_HANDLED == secondMongoSocketAction);
     //TEST : check if general mongo action not taken
     CPPUNIT_ASSERT(NO_ACTION_TAKEN == mongoAction);

     //
     // throw connect exception
     // there are 2 registered custom connect handlers
     // check if specific connect registered handlers are called
     //
     try
     {
       throw mongo::ConnectException("");
     }
     catch(mongo::ConnectException& e)
     {
       OsExceptionHandler::catch_global();
     }
     //TEST : check if first socket action taken
     CPPUNIT_ASSERT(EXCEPTION_HANDLED == mongoConnectAction);
     //TEST : check if second socket action taken
     CPPUNIT_ASSERT(EXCEPTION_HANDLED == secondMongoConnectAction);
     //TEST : check if general mongo action not taken
     CPPUNIT_ASSERT(NO_ACTION_TAKEN == mongoAction);

     //
     // throw mongo general exception exception
     // there are 2 registered custom general handlers
     // check if specific general registered handlers are called
     //
     try
     {
       throw mongo::DBException("", 0);
     }
     catch(mongo::DBException& e)
     {
       OsExceptionHandler::catch_global();
     }
     //TEST : check if custom handlers actions were taken
     CPPUNIT_ASSERT(EXCEPTION_HANDLED == mongoAction);
     CPPUNIT_ASSERT(EXCEPTION_HANDLED == secondMongoAction);
   }

   void checkNoSpecificHandlers()
   {
     reInitTestVariables();
     OsExceptionHandler::clearHandlers();

     // register custom mongo general handler
     ExceptionHandler customMongoHandler = static_cast<ExceptionHandler>(boost::bind(&OsExceptionHandlerTest::phonyMongoExceptionHandler, this, _1));
     OsExceptionHandler::instance().registerHandler(MONGO_EXCEPTION, MONGO_GENERAL_EXCEPTION, customMongoHandler);

     //
     // throw a mongo socket exception
     // the default mongo handler should be called instead of the specific one
     // since there is no specific mongo socket handler registered
     //
     try
     {
       throw mongo::SocketException(mongo::SocketException::SEND_ERROR, "");
     }
     catch(mongo::SocketException& e)
     {
       OsExceptionHandler::catch_global();
     }

     //TEST: check if corresponding action was taken
     // since there is no mongo socket exception handler the general one should be called
     CPPUNIT_ASSERT(EXCEPTION_HANDLED == mongoAction);
     CPPUNIT_ASSERT(NO_ACTION_TAKEN == mongoSocketAction);

     reInitTestVariables();

     //
     // throw a mongo connect exception
     // the default mongo handler should be called instead of the specific one
     // since there is no specific mongo connect handler registered
     //
     try
     {
       throw mongo::ConnectException(mongo::ConnectException(""));
     }
     catch(mongo::ConnectException& e)
     {
       OsExceptionHandler::catch_global();
     }
     //TEST: check if corresponding action was taken
     // since there is no mongo socket exception handler the general one should be called
     CPPUNIT_ASSERT(EXCEPTION_HANDLED == mongoAction);
     CPPUNIT_ASSERT(NO_ACTION_TAKEN == mongoConnectAction);


     reInitTestVariables();
     OsExceptionHandler::clearHandlers();

     //register custom std handler
     ExceptionHandler customStdHandler = static_cast<ExceptionHandler>(boost::bind(&OsExceptionHandlerTest::phonyStdExceptionHandler, this, _1));
     OsExceptionHandler::instance().registerHandler(STD_EXCEPTION, STD_GENERAL_EXCEPTION, customStdHandler);

     //
     // throw mongo socket exception
     // check if corresponding action was taken
     // since there is no mongo socket or mongo general exception handler the
     // std general one should be called
     //
     try
     {
       throw mongo::SocketException(mongo::SocketException::SEND_ERROR, "");
     }
     catch(mongo::SocketException& e)
     {
       OsExceptionHandler::catch_global();
     }

     //TEST : check if std general handler called
     CPPUNIT_ASSERT(EXCEPTION_HANDLED == stdAction);
     //TEST : check if mongo  general handler not called
     CPPUNIT_ASSERT(NO_ACTION_TAKEN == mongoAction);
     //TEST : check if mongo socket handler not called
     CPPUNIT_ASSERT(NO_ACTION_TAKEN == mongoSocketAction);

     //
     // throw mongo connect exception
     // check if corresponding action was taken
     // since there is no mongo connect or mongo general exception handler the
     // std general one should be called
     //
     try
     {
       throw mongo::ConnectException(mongo::ConnectException(""));
     }
     catch(mongo::ConnectException& e)
     {
       OsExceptionHandler::catch_global();
     }
     //TEST : check if std general handler called
     CPPUNIT_ASSERT(EXCEPTION_HANDLED == stdAction);
     //TEST : check if mongo connect handler not called
     CPPUNIT_ASSERT(NO_ACTION_TAKEN == mongoConnectAction);
     //TEST : check if mongo general handler not called
     CPPUNIT_ASSERT(NO_ACTION_TAKEN == mongoAction);
   }

   void getHandlersTest()
   {
     HandlerVector handlers;
     //
     // try to get the std general associated handlers
     // the call shoudl return OS_SUCCESS since the handlers are registered
     //
     OsStatus ret = OsExceptionHandler::getHandlers(STD_GENERAL_EXCEPTION, OsExceptionHandler::_stdHandlersContainer, handlers);
     //TEST: check if vector retrieved
     CPPUNIT_ASSERT(OS_SUCCESS == ret);

     //
     // clear registered handler and try to get vector of handlers
     // the call should fail since the are no registered handlers
     //
     OsExceptionHandler::clearHandlers();
     ret = OsExceptionHandler::getHandlers(STD_GENERAL_EXCEPTION, OsExceptionHandler::_stdHandlersContainer, handlers);
     CPPUNIT_ASSERT(OS_FAILED == ret);
   }

   void insertInContainerTest()
   {
     //clear handlers and manually insert a handler in container
     OsExceptionHandler::clearHandlers();

     ExceptionHandler customStdHandler = static_cast<ExceptionHandler>(boost::bind(&OsExceptionHandlerTest::phonyStdExceptionHandler, this, _1));

     OsExceptionHandler::insertInContainer(STD_GENERAL_EXCEPTION, customStdHandler, OsExceptionHandler::_stdHandlersContainer);

     //TEST: check number of entries in exception containers
     CPPUNIT_ASSERT(OS_SUCCESS == verifyContainersSize(1, 0, 0));
     //TEST: number of std handlers has to be 1
     CPPUNIT_ASSERT(OS_SUCCESS == verifyNumberOfHandlers(OsExceptionHandler::_stdHandlersContainer.at(STD_GENERAL_EXCEPTION), 1));
   }

 };

CPPUNIT_TEST_SUITE_REGISTRATION(OsExceptionHandlerTest);
