//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#ifdef TEST
#include <assert.h>
#include "utl/UtlMemCheck.h"
#endif //TEST
#include <stdio.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES
#include "siptest/AuthCommand.h"
#include "siptest/CommandProcessor.h"
#include "net/SipLine.h"
#include "net/NetMd5Codec.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
static const
char UsageMsg[] =
"   <identity-sip-url> <username> <realm> <password> (add this identitiy - explicit user and realm)\n"
"     | <identity-sip-url> <password>                    (add this identitiy - get user and realm from url)\n"
"     | <identity-sip-url>                               (delete this identity)\n"
;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
AuthCommand::AuthCommand(SipLineMgr* lineMgr) :
  mLineMgr( *lineMgr )
{
#ifdef TEST
  if (!sIsTested)
    {
      sIsTested = true;
      test();
    }
#endif //TEST
}


// Destructor
AuthCommand::~AuthCommand()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
AuthCommand&
AuthCommand::operator=(const AuthCommand& rhs)
{
  if (this == &rhs)            // handle the assignment to self case
    return *this;

  return *this;
}

int AuthCommand::execute(int argc, char* argv[])
{
  int commandStatus = CommandProcessor::COMMAND_FAILED;
  Url      identity;
  UtlString user;
  UtlString realm;
  UtlString password;

  switch ( argc )
    {
    case 2:
      // Delete Identity
      identity = argv[1];

      printf( "Deleting identity '%s'\n", argv[1] );
      mLineMgr.deleteLine( identity );
      break;

    case 3: // <identity-url> <password>
    case 5: // <identity-url> <user> <realm> <password>
      {
        // Add Identity
        identity = argv[1];

        if ( 3 == argc )
          {
            identity.getHostAddress( realm );
            identity.getUserId( user );
            user.append("@");
            user.append(realm);
            password = argv[2];
          }
        else
          {
            user     = argv[2];
            realm    = argv[3];
            password = argv[4];
          }

        SipLine  line( identity // user entered url
                      ,identity // identity url
                      ,user     // user
                      ,TRUE     // visible
                      ,SipLine::LINE_STATE_PROVISIONED
                      ,TRUE     // auto enable
                      ,FALSE    // use call handling
                      );

        mLineMgr.addLine( line );

        UtlString cred_input;
        UtlString cred_digest;

        cred_input.append( user );
        cred_input.append( ":" );
        cred_input.append( realm );
        cred_input.append( ":" );
        cred_input.append( password );

        NetMd5Codec::encode( cred_input.data(), cred_digest );

        printf("Adding identity '%s': user='%s' realm='%s' password='%s'\n"
               "                      H(A1)='%s'\n"
               ,identity.toString().data(), user.data(), realm.data()
               ,password.data(), cred_digest.data()
               );

        commandStatus = (   mLineMgr.addCredentialForLine( identity, realm, user, cred_digest
                                                          ,HTTP_DIGEST_AUTHENTICATION
                                                          )
                         )
          ? CommandProcessor::COMMAND_SUCCESS
          : CommandProcessor::COMMAND_FAILED
          ;
      }
      break;

    default:
      fprintf( stderr, "%s: Invalid number of arguments\n", argv[0] );
      fprintf( stderr, "%s%s", argv[0], UsageMsg );
      break;
    }

  return(commandStatus);
}

/* ============================ ACCESSORS ================================= */

void AuthCommand::getUsage(const char* commandName, UtlString* usage) const
{
  Command::getUsage(commandName, usage);
  usage->append( UsageMsg );
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

#ifdef TEST

// Set to true after the tests have been executed once
bool AuthCommand::sIsTested = false;

// Test this class by running all of its assertion tests
void AuthCommand::test()
{

  UtlMemCheck* pUtlMemCheck = 0;
  pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

  testCreators();
  testManipulators();
  testAccessors();
  testInquiry();

  assert(pUtlMemCheck->delta() == 0);    // check for memory leak
  delete pUtlMemCheck;
}

// Test the creators (and destructor) methods for the class
void AuthCommand::testCreators()
{
  UtlMemCheck* pUtlMemCheck  = 0;


  pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

  // test the default constructor (if implemented)
  // test the copy constructor (if implemented)
  // test other constructors (if implemented)
  //    if a constructor parameter is used to set information in an ancestor
  //       class, then verify it gets set correctly (i.e., via ancestor
  //       class accessor method.
  // test the destructor
  //    if the class contains member pointer variables, verify that the
  //    pointers are getting scrubbed.

  assert(pUtlMemCheck->delta() == 0);    // check for memory leak
  delete pUtlMemCheck;
}

// Test the manipulator methods
void AuthCommand::testManipulators()
{
  UtlMemCheck* pUtlMemCheck  = 0;

  pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

  // test the assignment method (if implemented)
  // test the other manipulator methods for the class

  assert(pUtlMemCheck->delta() == 0);    // check for memory leak
  delete pUtlMemCheck;
}

// Test the accessor methods for the class
void AuthCommand::testAccessors()
{
  UtlMemCheck* pUtlMemCheck  = 0;

  pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

  // body of the test goes here

  assert(pUtlMemCheck->delta() == 0);    // check for memory leak
  delete pUtlMemCheck;
}

// Test the inquiry methods for the class
void AuthCommand::testInquiry()
{
  UtlMemCheck* pUtlMemCheck  = 0;


  pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

  // body of the test goes here

  assert(pUtlMemCheck->delta() == 0);    // check for memory leak
  delete pUtlMemCheck;
}

#endif //TEST

/* ============================ FUNCTIONS ================================= */
