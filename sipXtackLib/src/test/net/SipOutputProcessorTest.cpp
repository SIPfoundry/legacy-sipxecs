//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
//////////////////////////////////////////////////////////////////////////////

#include <sys/time.h>
#include <unistd.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <utl/UtlSList.h>
#include <utl/UtlString.h>
#include <os/OsDefs.h>
#include <net/SipMessage.h>
#include <net/SipOutputProcessor.h>
#include <net/SipUserAgent.h>


// Class used to store SIP message, IP address and port
// information provided by an  output processor callback call.
class CallbackTrace : public UtlContainable
{
private:
   SipMessage mMessage;
   UtlString  mAddress;
   int        mPort;
   unsigned long long  mTimestampInMillisecs;
   
public:
   CallbackTrace(){}
   
   CallbackTrace( SipMessage& message,
                const char* address,
                int port ) : 
      mMessage( message ),
      mAddress( address ),
      mPort( port )
   {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      mTimestampInMillisecs = tv.tv_sec*1000ull + tv.tv_usec / 1000ull;    
   }

   long long getTimeStamp( void ) const
   {
      return mTimestampInMillisecs;
   }
   
   void setMessage( SipMessage& message )
   {
      mMessage = message;
   }
   
   SipMessage& getMessage( void ) 
   {
      return mMessage;
   }
   
   void setAddress( UtlString address )
   {
      mAddress = address;
   }
   
   const UtlString& getAddress( void ) const
   {
      return mAddress;
   }
   
   void setPort( int port )
   {
      mPort = port;
   }
   
   int getPort( void ) const
   {
      return mPort;
   }
   
   bool operator==( const CallbackTrace& rhs ) const
   {
      UtlString localMessage, rhsMessage;
      ssize_t       localLen,     rhsLen;
      
      // this method is often used to compare a message
      // captured by the output processor against a reference message
      // sent.  Given that, headers that have non-deterministric or 
      // hard-to-predict values are not considered when doing the 
      // comparison.  These headers are Date: Via: and User-Agent:.
      // We also remove the To-tag, as it is not predictable.
      SipMessage tmpMessage1( mMessage );
      tmpMessage1.removeTopVia();
      tmpMessage1.removeHeader( HTTP_DATE_FIELD, 0 );
      tmpMessage1.removeHeader( HTTP_USER_AGENT_FIELD, 0 );

      UtlString toField;
      tmpMessage1.getToField(&toField);
      Url toUri(toField, Url::NameAddr);
      toUri.removeFieldParameter("tag");
      // Force <...> to be used when serializing toUri.
      toUri.includeAngleBrackets();
      toUri.toString(toField);
      tmpMessage1.setRawToField(toField);

      SipMessage tmpMessage2( rhs.mMessage );
      tmpMessage2.removeTopVia();
      tmpMessage2.removeHeader( HTTP_DATE_FIELD, 0 );
      tmpMessage2.removeHeader( HTTP_USER_AGENT_FIELD, 0 );
      
      tmpMessage2.getToField(&toField);
      toUri.removeFieldParameter("tag");
      // Force <...> to be used when serializing toUri.
      toUri.includeAngleBrackets();
      toUri.toString(toField);
      tmpMessage2.setRawToField(toField);

      tmpMessage1.getBytes( &localMessage, &localLen );
      tmpMessage2.getBytes( &rhsMessage, &rhsLen );
      
      return ( ( localLen == rhsLen )                        &&
               ( localMessage.compareTo( rhsMessage ) == 0 ) &&
               ( mAddress.compareTo( rhs.mAddress ) == 0 )   &&
               ( mPort == rhs.mPort ) );
   }
   virtual UtlContainableType getContainableType() const { return "CallbackTrace"; }

   virtual unsigned hash() const{ return directHash(); }
   
   virtual int compareTo(UtlContainable const *rhsContainable ) const
   {
      CallbackTrace* rhs = (CallbackTrace*)rhsContainable;
      if( mPort != rhs->mPort )
      {
         return ( mPort < rhs->mPort ) ? -1 : 1;  
      }
      else
      {
         UtlString thisCallbackTraceString;
         UtlString rhsCallbackTraceString;
         ssize_t len;
         
         mMessage.getBytes( &thisCallbackTraceString, &len );
         thisCallbackTraceString.append( mAddress );
         
         rhs->mMessage.getBytes( &rhsCallbackTraceString, &len );
         rhsCallbackTraceString.append( rhs->mAddress );

         return( thisCallbackTraceString.compareTo( rhsCallbackTraceString ) );
      }
   }
};

// Class that represents an Output Processor
// This class saves all the information it receives via
// the output processor callbacks into a list for later retrieval
// by unit test cases.
class OutputProcessorFixture : public SipOutputProcessor
{
private:
   UtlSList      mCallbackTraceList;
   bool          mRestDuringCallback;

public:
   OutputProcessorFixture( uint prio = 0, bool restDuringCallback = false ) : 
      SipOutputProcessor( prio ),
   mRestDuringCallback( restDuringCallback ){}
   
   void handleOutputMessage( SipMessage& message,
                             const char* address,
                             int port )
   {
      if( mRestDuringCallback )
      {
         sleep(1);
      }
      UtlContainable* pCallbackTrace = new CallbackTrace( message, address, port );
      mCallbackTraceList.append( pCallbackTrace );
   }
   
   // function that waits no more than 'maxWaitInSecs' for
   // 'numberOfMessages' received by the processor
   bool waitForMessages( size_t numberOfMessages, int maxWaitInSecs )
   {
      // poll every second
      int index;
      bool bMessagesReceived = false;
      
      for( index = 0; index < maxWaitInSecs; index++ )
      {
         if( mCallbackTraceList.entries() >= numberOfMessages )
         {
            bMessagesReceived = true;
            break;
         }
         sleep( 1 );
      }
      return bMessagesReceived;
   }

   CallbackTrace& getLastStoredCallbackTrace( void ) 
   {
      return *( (CallbackTrace*)(mCallbackTraceList.last()) );
   }
   
   CallbackTrace& getStoredCallbackTrace( size_t index )
   {
      return *( (CallbackTrace*)(mCallbackTraceList.at( index )) );
   }

   void resetCallbackTrace( void )
   {
      mCallbackTraceList.destroyAll();
   }
   
   int getCallbackCount( void ) const
   {
      return mCallbackTraceList.entries();
   }
};

// class that implements a user agent that
// uses From header user part of incoming INIVTEs
// to decide which response to generate.
// This trick effectively enables a unit
// test to select which response this user
// agent is going to generate therefore enabling
// the testing of the SIP output processor with a 
// variety of responses.
class SipMessageResponder : public OsServerTask
{
  public:
     SipUserAgent *pUserAgent;

     SipMessageResponder()
        : OsServerTask("SipMessageResponder")
     {
        start();
        pUserAgent = new SipUserAgent(     8886
                                          ,8886
                                          ,8887
                                          ,"127.0.0.1"  // default publicAddress
                                          ,NULL         // default defaultUser
                                          ,"127.0.0.1"  // default defaultSipAddress
                                          ,"127.0.0.1:12345"); // proxy messages to neverland 
        
        OsMsgQ* queue = getMessageQueue();
        pUserAgent->addMessageObserver(*queue,
                                        "", // All methods
                                        TRUE, // Requests,
                                        FALSE, //Responses,
                                        TRUE, //Incoming,
                                        FALSE, //OutGoing,
                                        "", //eventName,
                                        NULL, //   SipSession* pSession,
                                        NULL); //processorData)
        
        pUserAgent->start();
     }
        
     ~SipMessageResponder()
     {
        waitUntilShutDown();
        pUserAgent->shutdown();
        delete pUserAgent;
     }

     virtual UtlBoolean handleMessage(OsMsg& eventMessage)
     {
        UtlBoolean bHandled = FALSE;
        int msgType = eventMessage.getMsgType();
        if ( msgType == OsMsg::PHONE_APP )
        {
           SipMessageEvent* sipMsgEvent = dynamic_cast<SipMessageEvent*>(&eventMessage);
           SipMessage* sipRequest = const_cast<SipMessage*>(sipMsgEvent->getMessage());

           if ( !sipRequest->isResponse() )
           {
              UtlString method;
              sipRequest->getRequestMethod( &method );
              if( method.compareTo( SIP_INVITE_METHOD ) == 0 )
              {
                 // user part of 'From' header is used to encode which response to generate
                 Url fromUrl;
                 UtlString username;
                 sipRequest->getFromUrl( fromUrl );
                 fromUrl.getUserId( username );
                 SipMessage response;
                 
                 if( username.compareTo( "180") == 0 )
                 {
                    response.setInviteRingingData( sipRequest );                 
                 }
                 else if( username.compareTo( "200") == 0  )
                 {
                    response.setOkResponseData( sipRequest ); 
                 }
                 else if( username.compareTo( "301") == 0  )
                 {
                    response.setResponseData(sipRequest, SIP_PERMANENT_MOVE_CODE, SIP_PERMANENT_MOVE_TEXT );
                 }
                 else if( username.compareTo( "400") == 0  )
                 {
                    response.setRequestBadRequest( sipRequest ); 
                 }
                 else if( username.compareTo( "501") == 0  )
                 {
                    response.setRequestUnimplemented( sipRequest ); 
                 }
                 else if( username.compareTo( "603") == 0  )
                 {
                    response.setResponseData( sipRequest, SIP_DECLINE_CODE, SIP_DECLINE_TEXT );
                 }
                 pUserAgent->send(response);
                 bHandled = TRUE;
              }
           }
        }
        return(bHandled);
     }
};

// Unittest for Sip Output Processor functionality
class SipOutputProcessorTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SipOutputProcessorTest);
      CPPUNIT_TEST(testAddRemoveProcessors);
      CPPUNIT_TEST(testProcessInviteRequests);
//      CPPUNIT_TEST(testProcessAckRequests);
      CPPUNIT_TEST(testProcessByeRequests);
      CPPUNIT_TEST(testProcessOptionsRequests);
      CPPUNIT_TEST(testProcessRegisterRequests);
      CPPUNIT_TEST(testProcessInfoRequests);
      CPPUNIT_TEST(testProcessPrackRequests);
      CPPUNIT_TEST(testProcessSubscribeRequests);
      CPPUNIT_TEST(testProcessNotifyRequests);
      CPPUNIT_TEST(testProcessUpdateRequests);
      CPPUNIT_TEST(testProcessMessageRequests);
      CPPUNIT_TEST(testProcessReferRequests);
      CPPUNIT_TEST(testProcessPublishRequests);
      CPPUNIT_TEST(testProcess1xxResponses);
      CPPUNIT_TEST(testProcess2xxResponses);
      CPPUNIT_TEST(testProcess3xxResponses);
      CPPUNIT_TEST(testProcess4xxResponses);
      CPPUNIT_TEST(testProcess5xxResponses);
      CPPUNIT_TEST(testProcess6xxResponses);
      CPPUNIT_TEST(testPriorityOrdering);
      CPPUNIT_TEST_SUITE_END();

public:
   SipMessageResponder* sipResponder;
   
   void setUp()
   {
      // UA that will act as the receiver of test messages
      // generated by the SIP user agent under test to  
      // prevent it from retransmitting packets sent over UDP.
      // Retransmitted packets would be seen by the processor which would
      // cause tests to fail in error.
      sipResponder = new SipMessageResponder();
   }

   void tearDown()
   {
      delete sipResponder;
   }
      
   void testAddRemoveProcessors()
   {
      OutputProcessorFixture* pProcessor1 = new OutputProcessorFixture();
      OutputProcessorFixture* pProcessor2 = new OutputProcessorFixture();
      OutputProcessorFixture* pProcessor3 = new OutputProcessorFixture();
      
      SipUserAgent sipUA( 7777
                         ,7777
                         ,7778
                         ,"127.0.0.1:7777" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to receivingSipUA
      
      sipUA.start();

      const char* simpleMessage = 
          "INVITE sip:1@192.168.0.6 SIP/2.0\r\n"
          "From: <sip:200@10.1.1.144;user=phone>;tag=bbb\r\n"
          "To: <sip:3000@192.168.0.3:3000;user=phone>\r\n"
          "Call-Id: 8\r\n"
          "Cseq: 1 INVITE\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      SipMessage testMsg( simpleMessage, strlen( simpleMessage ) );

      // add three processors, verify that they all get the message 
      sipUA.addSipOutputProcessor( pProcessor1 );
      sipUA.addSipOutputProcessor( pProcessor2 );
      sipUA.addSipOutputProcessor( pProcessor3 );

      sipUA.send( testMsg );
      
      CallbackTrace referenceTrace( testMsg, "127.0.0.1", 8886 );
      CPPUNIT_ASSERT( pProcessor1->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor1->getLastStoredCallbackTrace() == referenceTrace );
      
      CPPUNIT_ASSERT( pProcessor2->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor2->getLastStoredCallbackTrace() == referenceTrace );

      CPPUNIT_ASSERT( pProcessor3->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor3->getLastStoredCallbackTrace() == referenceTrace );
      
      // remove one processor, verify that only the two remaining get the message
      sipUA.removeSipOutputProcessor( pProcessor2 );

      const char* simpleMessage2 = 
          "INVITE sip:2@192.168.0.6 SIP/2.0\r\n"
          "From: <sip:200@10.1.1.144;user=phone>;tag=bbb\r\n"
          "To: <sip:3000@192.168.0.3:3000;user=phone>\r\n"
          "Call-Id: 1\r\n"
          "Cseq: 1 INVITE\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      SipMessage testMsg2( simpleMessage2, strlen( simpleMessage2 ) );

      sipUA.send( testMsg2 );
      
      CallbackTrace referenceTrace2( testMsg2, "127.0.0.1", 8886 );
   
      CPPUNIT_ASSERT( pProcessor1->getCallbackCount() == 2 );
      CPPUNIT_ASSERT( pProcessor1->getLastStoredCallbackTrace() == referenceTrace2 );
      
      CPPUNIT_ASSERT( pProcessor2->getCallbackCount() == 1 );

      CPPUNIT_ASSERT( pProcessor3->getCallbackCount() == 2 );
      CPPUNIT_ASSERT( pProcessor3->getLastStoredCallbackTrace() == referenceTrace2 );
      
      // remove one more processor, verify that only the remaining one gets the message
      sipUA.removeSipOutputProcessor( pProcessor1 );

      const char* simpleMessage3 = 
          "INVITE sip:3@192.168.0.6 SIP/2.0\r\n"
          "From: <sip:200@10.1.1.144;user=phone>;tag=bbb\r\n"
          "To: <sip:3000@192.168.0.3:3000;user=phone>\r\n"
          "Call-Id: 3\r\n"
          "Cseq: 1 INVITE\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      SipMessage testMsg3( simpleMessage3, strlen( simpleMessage3 ) );

      sipUA.send( testMsg3 );
      
      CallbackTrace referenceTrace3( testMsg3, "127.0.0.1", 8886 );
   
      CPPUNIT_ASSERT( pProcessor1->getCallbackCount() == 2 );
      
      CPPUNIT_ASSERT( pProcessor2->getCallbackCount() == 1 );

      CPPUNIT_ASSERT( pProcessor3->getCallbackCount() == 3 );
      CPPUNIT_ASSERT( pProcessor3->getLastStoredCallbackTrace() == referenceTrace3 );
      
      // remove remaining  processor, verify that nobody gets the message
      sipUA.removeSipOutputProcessor( pProcessor3 );

      const char* simpleMessage4 = 
          "INVITE sip:4@192.168.0.6 SIP/2.0\r\n"
          "From: <sip:200@10.1.1.144;user=phone>;tag=bbb\r\n"
          "To: <sip:3000@192.168.0.3:3000;user=phone>\r\n"
          "Call-Id: 4\r\n"
          "Cseq: 1 INVITE\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      SipMessage testMsg4( simpleMessage4, strlen( simpleMessage4 ) );

      sipUA.send( testMsg4 );
      
      CallbackTrace referenceTrace4( testMsg4, "127.0.0.1", 8886 );
   
      CPPUNIT_ASSERT( pProcessor1->getCallbackCount() == 2 );
      
      CPPUNIT_ASSERT( pProcessor2->getCallbackCount() == 1 );

      CPPUNIT_ASSERT( pProcessor3->getCallbackCount() == 3 );
      
      // re-introduce one processor and verify that it gets the message
      sipUA.addSipOutputProcessor( pProcessor2 );

      const char* simpleMessage5 = 
          "INVITE sip:5@192.168.0.6 SIP/2.0\r\n"
          "From: <sip:200@10.1.1.144;user=phone>;tag=bbb\r\n"
          "To: <sip:3000@192.168.0.3:3000;user=phone>\r\n"
          "Call-Id: 5\r\n"
          "Cseq: 1 INVITE\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      SipMessage testMsg5( simpleMessage5, strlen( simpleMessage5 ) );

      sipUA.send( testMsg5 );
      
      CallbackTrace referenceTrace5( testMsg5, "127.0.0.1", 8886 );
   
      CPPUNIT_ASSERT( pProcessor1->getCallbackCount() == 2 );
      
      CPPUNIT_ASSERT( pProcessor2->getCallbackCount() == 2 );
      CPPUNIT_ASSERT( pProcessor2->getLastStoredCallbackTrace() == referenceTrace5 );

      CPPUNIT_ASSERT( pProcessor3->getCallbackCount() == 3 );

      sipUA.removeSipOutputProcessor( pProcessor2 );
      sipUA.shutdown();
        
      delete pProcessor1;
      delete pProcessor2;
      delete pProcessor3;
   }

   void testProcessInviteRequests()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      
      SipUserAgent sipUA( 7777
                         ,7777
                         ,7778
                         ,"127.0.0.1:7777" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to receivingSipUA
      sipUA.start();
      sipUA.addSipOutputProcessor( pProcessor );
  
      const char* inviteMessage =
          "INVITE sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:200@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 INVITE\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      SipMessage inviteSipMsg(inviteMessage, strlen(inviteMessage));

      sipUA.send( inviteSipMsg );
      CallbackTrace referenceTrace1( inviteSipMsg, "127.0.0.1", 8886 );
   
      CPPUNIT_ASSERT( pProcessor->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor->getLastStoredCallbackTrace() == referenceTrace1 );
      sipUA.removeSipOutputProcessor( pProcessor );
      sipUA.shutdown();
      delete( pProcessor );
   }

   void testProcessAckRequests()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      
      SipUserAgent sipUA( 7777
                         ,7777
                         ,7778
                         ,"127.0.0.1:7777" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to receivingSipUA
      sipUA.start();
      sipUA.addSipOutputProcessor( pProcessor );
   
      const char* ackMessage =
          "ACK sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:200@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 ACK\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage ackSipMsg(ackMessage, strlen(ackMessage));

      sipUA.send( ackSipMsg );
      CallbackTrace referenceTrace2( ackSipMsg, "127.0.0.1", 8886 );
   
      CPPUNIT_ASSERT( pProcessor->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor->getLastStoredCallbackTrace() == referenceTrace2 );
      sipUA.removeSipOutputProcessor( pProcessor );
      sipUA.shutdown();
      delete( pProcessor );
   }     

   void testProcessByeRequests()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      
      SipUserAgent sipUA( 7777
                         ,7777
                         ,7778
                         ,"127.0.0.1:7777" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to receivingSipUA
      sipUA.start();
      sipUA.addSipOutputProcessor( pProcessor );
      const char* byeMessage =
          "BYE sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:200@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 BYE\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage byeSipMsg(byeMessage, strlen(byeMessage));
      
      sipUA.send( byeSipMsg );
      CallbackTrace referenceTrace3( byeSipMsg, "127.0.0.1", 8886 );
   
      CPPUNIT_ASSERT( pProcessor->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor->getLastStoredCallbackTrace() == referenceTrace3 );
      sipUA.removeSipOutputProcessor( pProcessor );
      sipUA.shutdown();
      delete( pProcessor );
   }
   
   void testProcessOptionsRequests()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      
      SipUserAgent sipUA( 7777
                         ,7777
                         ,7778
                         ,"127.0.0.1:7777" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to receivingSipUA
      sipUA.start();
      sipUA.addSipOutputProcessor( pProcessor );
      const char* optionsMessage =
          "OPTIONS sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:200@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 OPTIONS\r\n"
          "Max-Forwards: 20\r\n"
          "Supported: replaces\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage optionsSipMsg(optionsMessage, strlen(optionsMessage));
      sipUA.send( optionsSipMsg );
      CallbackTrace referenceTrace5( optionsSipMsg, "127.0.0.1", 8886 );
   
      CPPUNIT_ASSERT( pProcessor->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor->getLastStoredCallbackTrace() == referenceTrace5 );
      sipUA.removeSipOutputProcessor( pProcessor );
      sipUA.shutdown();
      delete( pProcessor );
   }

   void testProcessRegisterRequests()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      
      SipUserAgent sipUA( 7777
                         ,7777
                         ,7778
                         ,"127.0.0.1:7777" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to receivingSipUA
      sipUA.start();
      sipUA.addSipOutputProcessor( pProcessor );
   
      const char* registerMessage =
          "REGISTER sip:sipx.local SIP/2.0\r\n"
          "From: <sip:200@sipx.local>;tag=94b99ae0-2f816f01-13c4-be5-50f2a8d5-be5\r\n"
          "To: <sip:test@sipx.local>\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 REGISTER\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: test@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage registerSipMsg(registerMessage, strlen(registerMessage));
      sipUA.send( registerSipMsg );
      CallbackTrace referenceTrace6( registerSipMsg, "127.0.0.1", 8886 );
   
      CPPUNIT_ASSERT( pProcessor->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor->getLastStoredCallbackTrace() == referenceTrace6 );
      sipUA.removeSipOutputProcessor( pProcessor );
      sipUA.shutdown();
      delete( pProcessor );
   }

   void testProcessInfoRequests()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      
      SipUserAgent sipUA( 7777
                         ,7777
                         ,7778
                         ,"127.0.0.1:7777" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to receivingSipUA
      sipUA.start();
      sipUA.addSipOutputProcessor( pProcessor );
   
      const char* infoMessage =
          "INFO sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:200@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 INFO\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Supported: timer\r\n"
          "Content-Type: application/dtmf-relay\r\n"
          "Content-Length: 26\r\n"
          "\r\n"
          "Signal= 1\r\n"
          "Duration= 160\r\n"
          "\r\n";

      SipMessage infoSipMsg(infoMessage, strlen(infoMessage));
      sipUA.send( infoSipMsg );
      CallbackTrace referenceTrace7( infoSipMsg, "127.0.0.1", 8886 );
   
      CPPUNIT_ASSERT( pProcessor->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor->getLastStoredCallbackTrace() == referenceTrace7 );
      sipUA.removeSipOutputProcessor( pProcessor );
      sipUA.shutdown();
      delete( pProcessor );
   }
   
   void testProcessPrackRequests()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      
      SipUserAgent sipUA( 7777
                         ,7777
                         ,7778
                         ,"127.0.0.1:7777" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to receivingSipUA
      sipUA.start();
      sipUA.addSipOutputProcessor( pProcessor );   
      const char* prackMessage =
          "PRACK sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:200@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 PRACK\r\n"
          "Max-Forwards: 20\r\n"
          "RAck: 1 2 INVITE\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage prackSipMsg(prackMessage, strlen(prackMessage));
      sipUA.send( prackSipMsg );
      CallbackTrace referenceTrace8( prackSipMsg, "127.0.0.1", 8886 );
   
      CPPUNIT_ASSERT( pProcessor->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor->getLastStoredCallbackTrace() == referenceTrace8 );
      sipUA.removeSipOutputProcessor( pProcessor );
      sipUA.shutdown();
      delete( pProcessor );
   }
   
   void testProcessSubscribeRequests()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      
      SipUserAgent sipUA( 7777
                         ,7777
                         ,7778
                         ,"127.0.0.1:7777" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to receivingSipUA
      sipUA.start();
      sipUA.addSipOutputProcessor( pProcessor );
   
      const char* subscribeMessage =
          "SUBSCRIBE sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:200@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 SUBSCRIBE\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Expires: 3600\r\n"
          "Event: message-summary\r\n"
          "Max-Forwards: 20\r\n"
          "Supported: replaces\r\n"
          "Accept: application/simple-message-summary\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage subscribeSipMsg(subscribeMessage, strlen(subscribeMessage));
      sipUA.send( subscribeSipMsg );
      CallbackTrace referenceTrace9( subscribeSipMsg, "127.0.0.1", 8886 );
   
      CPPUNIT_ASSERT( pProcessor->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor->getLastStoredCallbackTrace() == referenceTrace9 );
      sipUA.removeSipOutputProcessor( pProcessor );
      sipUA.shutdown();
      delete( pProcessor );
   }

   void testProcessNotifyRequests()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      
      SipUserAgent sipUA( 7777
                         ,7777
                         ,7778
                         ,"127.0.0.1:7777" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to receivingSipUA
      sipUA.start();
      sipUA.addSipOutputProcessor( pProcessor );
      
      const char* notifyMessage =
          "NOTIFY sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:200@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 NOTIFY\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Event: message-summary\r\n"
          "Content-Type: application/simple-message-summary\r\n"
          "Content-Length: 50\r\n"
          "\r\n"
          "Messages-Waiting: no\r\n"
          "Voice-Message: 0/0 (0/0)\r\n"
          "\r\n";
      
      SipMessage notifySipMsg(notifyMessage, strlen(notifyMessage));
      sipUA.send( notifySipMsg );
      CallbackTrace referenceTrace10( notifySipMsg, "127.0.0.1", 8886 );
   
      CPPUNIT_ASSERT( pProcessor->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor->getLastStoredCallbackTrace() == referenceTrace10 );
      sipUA.removeSipOutputProcessor( pProcessor );
      sipUA.shutdown();
      delete( pProcessor );
   }
   
   void testProcessUpdateRequests()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      
      SipUserAgent sipUA( 7777
                         ,7777
                         ,7778
                         ,"127.0.0.1:7777" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to receivingSipUA
      sipUA.start();
      sipUA.addSipOutputProcessor( pProcessor );
      
      const char* updateMessage =
          "UPDATE sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:200@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 UPDATE\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage updateSipMsg(updateMessage, strlen(updateMessage));
      sipUA.send( updateSipMsg );
      CallbackTrace referenceTrace11( updateSipMsg, "127.0.0.1", 8886 );
   
      CPPUNIT_ASSERT( pProcessor->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor->getLastStoredCallbackTrace() == referenceTrace11 );
      sipUA.removeSipOutputProcessor( pProcessor );
      sipUA.shutdown();
      delete( pProcessor );
   }
   
   void testProcessMessageRequests()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      
      SipUserAgent sipUA( 7777
                         ,7777
                         ,7778
                         ,"127.0.0.1:7777" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to receivingSipUA
      sipUA.start();
      sipUA.addSipOutputProcessor( pProcessor );
      
      const char* messageMessage =
          "MESSAGE sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:200@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 MESSAGE\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage messageSipMsg(messageMessage, strlen(messageMessage));
      sipUA.send( messageSipMsg );
      CallbackTrace referenceTrace12( messageSipMsg, "127.0.0.1", 8886 );
   
      CPPUNIT_ASSERT( pProcessor->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor->getLastStoredCallbackTrace() == referenceTrace12 );
      sipUA.removeSipOutputProcessor( pProcessor );
      sipUA.shutdown();
      delete( pProcessor );
   }

   void testProcessReferRequests()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      
      SipUserAgent sipUA( 7777
                         ,7777
                         ,7778
                         ,"127.0.0.1:7777" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to receivingSipUA
      sipUA.start();
      sipUA.addSipOutputProcessor( pProcessor );
      
      const char* referMessage =
          "REFER sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:200@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 REFER\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage referSipMsg(referMessage, strlen(referMessage));
      sipUA.send( referSipMsg );
      CallbackTrace referenceTrace13( referSipMsg, "127.0.0.1", 8886 );
   
      CPPUNIT_ASSERT( pProcessor->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor->getLastStoredCallbackTrace() == referenceTrace13 );
      sipUA.removeSipOutputProcessor( pProcessor );
      sipUA.shutdown();
      delete( pProcessor );
   }

   void testProcessPublishRequests()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      
      SipUserAgent sipUA( 7777
                         ,7777
                         ,7778
                         ,"127.0.0.1:7777" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to receivingSipUA
      sipUA.start();
      sipUA.addSipOutputProcessor( pProcessor );
   
      const char* publishMessage =
          "PUBLISH sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:200@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 PUBLISH\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage publishSipMsg(publishMessage, strlen(publishMessage));
      sipUA.send( publishSipMsg );
      CallbackTrace referenceTrace14( publishSipMsg, "127.0.0.1", 8886 );
   
      CPPUNIT_ASSERT( pProcessor->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor->getLastStoredCallbackTrace() == referenceTrace14 );

      sipUA.removeSipOutputProcessor( pProcessor );
      sipUA.shutdown();
      delete( pProcessor );
   }
   
   void testProcess1xxResponses()
   {
      SipMessage cancelMsg;

      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      sipResponder->pUserAgent->addSipOutputProcessor( pProcessor );
      
      SipUserAgent sipUA( 9997
                         ,9997
                         ,9998
                         ,"127.0.0.1:9997" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to responder which will generate responses
      sipUA.start();
   // 180 Ringing
      const char* inviteFrom180Message =
          "INVITE sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:180@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
          "Cseq: 1 INVITE\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      SipMessage inviteFrom180Msg(inviteFrom180Message, strlen(inviteFrom180Message));
      pProcessor->resetCallbackTrace();
      sipUA.send( inviteFrom180Msg );
      CPPUNIT_ASSERT( pProcessor->waitForMessages( 2, 10 ) == true );

      const char * expected180Response = 
         "SIP/2.0 180 Ringing\r\n"
         "From: Sip Send <sip:180@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
         "To: sip:sipx.local\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
         "Cseq: 1 INVITE\r\n"
         "Via: SIP/2.0/UDP 127.0.0.1:9997:9997;branch=z9hG4bK-sipXecs-00e41469f4f11e314fefada30ec424d532fa\r\n"
         "Contact: <sip:127.0.0.1:8886>\r\n"
         "User-Agent: sipXecs/3.10.0 (Linux)\r\n"
         "Accept-Language: en\r\n"
         "Allow: INVITE, ACK, CANCEL, BYE, REFER, OPTIONS\r\n"  
         "Content-Length: 0\r\n"
         "\r\n";

      SipMessage expected180ResponseMsg(expected180Response, strlen(expected180Response) );
      CallbackTrace reference180( expected180ResponseMsg, "127.0.0.1", 9997 );
      
      CPPUNIT_ASSERT( pProcessor->getStoredCallbackTrace(1) == reference180 );

      cancelMsg.setCancelData( &inviteFrom180Msg );
      pProcessor->resetCallbackTrace();
      sipUA.send( cancelMsg );
      CPPUNIT_ASSERT( pProcessor->waitForMessages( 1, 10 ) == true );
      sipUA.shutdown();

      sipResponder->pUserAgent->removeSipOutputProcessor( pProcessor );
      delete( pProcessor );
   }
   
   void testProcess2xxResponses()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      sipResponder->pUserAgent->addSipOutputProcessor( pProcessor );
      
      SipUserAgent sipUA( 9997
                         ,9997
                         ,9998
                         ,"127.0.0.1:9997" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to rresponder which will generate responses
      sipUA.start();

   // 200 OK
      const char* inviteFrom200Message =
          "INVITE sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:200@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295c\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe874a\r\n"
          "Cseq: 1 INVITE\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      SipMessage inviteFrom200Msg(inviteFrom200Message, strlen(inviteFrom200Message));
      pProcessor->resetCallbackTrace();
      sipUA.send( inviteFrom200Msg );
      CPPUNIT_ASSERT( pProcessor->waitForMessages( 2, 10 ) == true );


      const char * expected200Response = 
         "SIP/2.0 200 OK\r\n"
         "From: Sip Send <sip:200@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295c\r\n"
         "To: sip:sipx.local\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe874a\r\n"
         "Cseq: 1 INVITE\r\n"
         "Via: SIP/2.0/UDP 127.0.0.1:9997;branch=z9hG4bK-sipXecs-0001e6b7ff528b085aebd671010b775f883a\r\n"
         "Contact: <sip:127.0.0.1:8886>\r\n"
         "Allow: INVITE, ACK, CANCEL, BYE, REFER, OPTIONS\r\n"
         "User-Agent: sipXecs/3.10.0 (Linux)\r\n"
         "Accept-Language: en\r\n"
         "Content-Length: 0\r\n"
         "\r\n";
      
      SipMessage expected200ResponseMsg(expected200Response, strlen(expected200Response) );
      CallbackTrace reference200( expected200ResponseMsg, "127.0.0.1", 9997 );
      
      CPPUNIT_ASSERT( pProcessor->getStoredCallbackTrace(1) == reference200 );
      sipUA.shutdown();

      sipResponder->pUserAgent->removeSipOutputProcessor( pProcessor );
      delete( pProcessor );
   }
   
   void testProcess3xxResponses()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      sipResponder->pUserAgent->addSipOutputProcessor( pProcessor );
      
      SipUserAgent sipUA( 9997
                         ,9997
                         ,9998
                         ,"127.0.0.1:9997" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to rresponder which will generate responses
      sipUA.start();

   // 301
      const char* inviteFrom301Message =
          "INVITE sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:301@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295c301\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe874a301\r\n"
          "Cseq: 1 INVITE\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      SipMessage inviteFrom301Msg(inviteFrom301Message, strlen(inviteFrom301Message));
      pProcessor->resetCallbackTrace();
      sipUA.send( inviteFrom301Msg );
      CPPUNIT_ASSERT( pProcessor->waitForMessages( 2, 10 ) == true );

      const char * expected301Response = 
         "SIP/2.0 301 Moved Permanently\r\n"
         "From: Sip Send <sip:301@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295c301\r\n"
         "To: sip:sipx.local\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe874a301\r\n"
         "Cseq: 1 INVITE\r\n"
         "Via: SIP/2.0/UDP 127.0.0.1:9997:9997;branch=z9hG4bK-sipXecs-0004e04e78227fd21bbf1b5248ab18f5ee28%b68e7965b130f3a33be3f383d668f39a\r\n"
         "Allow: INVITE, ACK, CANCEL, BYE, REFER, OPTIONS\r\n"
         "User-Agent: sipXecs/3.10.0 (Linux)\r\n"
         "Accept-Language: en\r\n"
         "Content-Length: 0\r\n"
         "\r\n";

      SipMessage expected301ResponseMsg(expected301Response, strlen(expected301Response) );
      CallbackTrace reference301( expected301ResponseMsg, "127.0.0.1", 9997 );
      
      CPPUNIT_ASSERT( pProcessor->getStoredCallbackTrace(1) == reference301 );
      sipUA.shutdown();

      sipResponder->pUserAgent->removeSipOutputProcessor( pProcessor );
      delete( pProcessor );
   }
   
   void testProcess4xxResponses()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      sipResponder->pUserAgent->addSipOutputProcessor( pProcessor );
      
      SipUserAgent sipUA( 9997
                         ,9997
                         ,9998
                         ,"127.0.0.1:9997" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to rresponder which will generate responses
      sipUA.start();
      
   // 400 
      const char* inviteFrom400Message =
          "INVITE sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:400@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295c400\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe874a400\r\n"
          "Cseq: 1 INVITE\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      SipMessage inviteFrom400Msg(inviteFrom400Message, strlen(inviteFrom400Message));
      pProcessor->resetCallbackTrace();
      sipUA.send( inviteFrom400Msg );
      CPPUNIT_ASSERT( pProcessor->waitForMessages( 2, 10 ) == true );

      const char * expected400Response = 
         "SIP/2.0 400 Bad Request\r\n"
         "From: Sip Send <sip:400@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295c400\r\n"
         "To: sip:sipx.local\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe874a400\r\n"
         "Cseq: 1 INVITE\r\n"
         "Via: SIP/2.0/UDP 127.0.0.1:9997:9997;branch=z9hG4bK-sipXecs-0004e04e78227fd21bbf1b5248ab18f5ee28%b68e7965b130f3a33be3f383d668f39a\r\n"
         "Allow: INVITE, ACK, CANCEL, BYE, REFER, OPTIONS\r\n"
         "User-Agent: sipXecs/3.10.0 (Linux)\r\n"
         "Accept-Language: en\r\n"
         "Content-Length: 0\r\n"
         "\r\n";

      SipMessage expected400ResponseMsg(expected400Response, strlen(expected400Response) );
      CallbackTrace reference400( expected400ResponseMsg, "127.0.0.1", 9997 );
      
      CPPUNIT_ASSERT( pProcessor->getStoredCallbackTrace(1) == reference400 );
      sipUA.shutdown();

      sipResponder->pUserAgent->removeSipOutputProcessor( pProcessor );
      delete( pProcessor );
   }
   
   void testProcess5xxResponses()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      sipResponder->pUserAgent->addSipOutputProcessor( pProcessor );
      
      SipUserAgent sipUA( 9997
                         ,9997
                         ,9998
                         ,"127.0.0.1:9997" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to rresponder which will generate responses
      sipUA.start();

   // 501 
      const char* inviteFrom501Message =
          "INVITE sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:501@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295c501\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe874a501\r\n"
          "Cseq: 1 INVITE\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      SipMessage inviteFrom501Msg(inviteFrom501Message, strlen(inviteFrom501Message));
      pProcessor->resetCallbackTrace();
      sipUA.send( inviteFrom501Msg );
      CPPUNIT_ASSERT( pProcessor->waitForMessages( 2, 10 ) == true );

      const char * expected501Response = 
         "SIP/2.0 501 Not Implemented\r\n"
         "From: Sip Send <sip:501@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295c501\r\n"
         "To: sip:sipx.local\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe874a501\r\n"
         "Cseq: 1 INVITE\r\n"
         "Via: SIP/2.0/UDP 127.0.0.1:9997:9997;branch=z9hG4bK-sipXecs-0004e04e78227fd21bbf1b5248ab18f5ee28%b68e7965b130f3a33be3f383d668f39a\r\n"
         "Allow: INVITE, ACK, CANCEL, BYE, REFER, OPTIONS\r\n"
         "User-Agent: sipXecs/3.10.0 (Linux)\r\n"
         "Accept-Language: en\r\n"
         "Content-Length: 0\r\n"
         "\r\n";

      SipMessage expected501ResponseMsg(expected501Response, strlen(expected501Response) );
      CallbackTrace reference501( expected501ResponseMsg, "127.0.0.1", 9997 );
      
      CPPUNIT_ASSERT( pProcessor->getStoredCallbackTrace(1) == reference501 );
      sipUA.shutdown();

      sipResponder->pUserAgent->removeSipOutputProcessor( pProcessor );
      delete( pProcessor );
   }

   void testProcess6xxResponses()
   {
      OutputProcessorFixture* pProcessor = new OutputProcessorFixture();
      sipResponder->pUserAgent->addSipOutputProcessor( pProcessor );
      
      SipUserAgent sipUA( 9997
                         ,9997
                         ,9998
                         ,"127.0.0.1:9997" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to rresponder which will generate responses
      sipUA.start();
      
   // 603 
      const char* inviteFrom603Message =
          "INVITE sip:sipx.local SIP/2.0\r\n"
          "To: sip:sipx.local\r\n"
          "From: Sip Send <sip:603@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295c603\r\n"
          "Call-ID: f88dfabce84b6a2787ef024a7dbe874a603\r\n"
          "Cseq: 1 INVITE\r\n"
          "Max-Forwards: 20\r\n"
          "Contact: me@127.0.0.1\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      SipMessage inviteFrom603Msg(inviteFrom603Message, strlen(inviteFrom603Message));
      pProcessor->resetCallbackTrace();
      sipUA.send( inviteFrom603Msg );
      CPPUNIT_ASSERT( pProcessor->waitForMessages( 2, 10 ) == true );

      const char * expected603Response = 
         "SIP/2.0 603 Declined\r\n"
         "From: Sip Send <sip:603@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295c603\r\n"
         "To: sip:sipx.local\r\n"
         "Call-Id: f88dfabce84b6a2787ef024a7dbe874a603\r\n"
         "Cseq: 1 INVITE\r\n"
         "Via: SIP/2.0/UDP 127.0.0.1:9997:9997;branch=z9hG4bK-sipXecs-0004e04e78227fd21bbf1b5248ab18f5ee28%b68e7965b130f3a33be3f383d668f39a\r\n"
         "Allow: INVITE, ACK, CANCEL, BYE, REFER, OPTIONS\r\n"
         "User-Agent: sipXecs/3.10.0 (Linux)\r\n"
         "Accept-Language: en\r\n"
         "Content-Length: 0\r\n"
         "\r\n";

      SipMessage expected603ResponseMsg(expected603Response, strlen(expected603Response) );
      CallbackTrace reference603( expected603ResponseMsg, "127.0.0.1", 9997 );
      
      CPPUNIT_ASSERT( pProcessor->getStoredCallbackTrace(1) == reference603 );
      sipUA.shutdown();

      sipResponder->pUserAgent->removeSipOutputProcessor( pProcessor );
      delete( pProcessor );
   }
   
   void testPriorityOrdering()
   {
      OutputProcessorFixture* pProcessor1  = new OutputProcessorFixture(1, true);
      OutputProcessorFixture* pProcessor2  = new OutputProcessorFixture(2, true);
      OutputProcessorFixture* pProcessor3  = new OutputProcessorFixture(3, true);
      OutputProcessorFixture* pProcessor4  = new OutputProcessorFixture(4, true);
      OutputProcessorFixture* pProcessor5  = new OutputProcessorFixture(5, true);
      OutputProcessorFixture* pProcessor6  = new OutputProcessorFixture(6, true);
      OutputProcessorFixture* pProcessor7  = new OutputProcessorFixture(7, true);
      OutputProcessorFixture* pProcessor8  = new OutputProcessorFixture(8, true);
      OutputProcessorFixture* pProcessor9  = new OutputProcessorFixture(9, true);
      OutputProcessorFixture* pProcessor10 = new OutputProcessorFixture(10, true);
      
      SipUserAgent sipUA( 7777
                         ,7777
                         ,7778
                         ,"127.0.0.1:7777" // default publicAddress
                         ,NULL             // default defaultUser
                         ,"127.0.0.1"      // default defaultSipAddress
                         ,"127.0.0.1:8886"); // proxy requests to receivingSipUA
      
      sipUA.start();

      const char* simpleMessage = 
          "INVITE sip:1@192.168.0.6 SIP/2.0\r\n"
          "From: <sip:200@10.1.1.144;user=phone>;tag=bbb\r\n"
          "To: <sip:3000@192.168.0.3:3000;user=phone>\r\n"
          "Call-Id: 8\r\n"
          "Cseq: 1 INVITE\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      SipMessage testMsg( simpleMessage, strlen( simpleMessage ) );

      // add the processors but not in their order of priority 
      sipUA.addSipOutputProcessor( pProcessor7 );
      sipUA.addSipOutputProcessor( pProcessor4 );
      sipUA.addSipOutputProcessor( pProcessor10 );
      sipUA.addSipOutputProcessor( pProcessor2 );
      sipUA.addSipOutputProcessor( pProcessor9 );
      sipUA.addSipOutputProcessor( pProcessor6 );
      sipUA.addSipOutputProcessor( pProcessor5 );
      sipUA.addSipOutputProcessor( pProcessor1 );
      sipUA.addSipOutputProcessor( pProcessor3 );
      sipUA.addSipOutputProcessor( pProcessor8 );

      sipUA.send( testMsg );
      
      CallbackTrace referenceTrace( testMsg, "127.0.0.1", 8886 );

      CPPUNIT_ASSERT( pProcessor1->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor1->getLastStoredCallbackTrace() == referenceTrace );
      CPPUNIT_ASSERT( pProcessor1->getLastStoredCallbackTrace().getTimeStamp() <  pProcessor2->getLastStoredCallbackTrace().getTimeStamp() );
      
      CPPUNIT_ASSERT( pProcessor2->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor2->getLastStoredCallbackTrace() == referenceTrace );
      CPPUNIT_ASSERT( pProcessor2->getLastStoredCallbackTrace().getTimeStamp() <  pProcessor3->getLastStoredCallbackTrace().getTimeStamp() );
      
      CPPUNIT_ASSERT( pProcessor3->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor3->getLastStoredCallbackTrace() == referenceTrace );
      CPPUNIT_ASSERT( pProcessor3->getLastStoredCallbackTrace().getTimeStamp() <  pProcessor4->getLastStoredCallbackTrace().getTimeStamp() );
      
      CPPUNIT_ASSERT( pProcessor4->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor4->getLastStoredCallbackTrace() == referenceTrace );
      CPPUNIT_ASSERT( pProcessor4->getLastStoredCallbackTrace().getTimeStamp() <  pProcessor5->getLastStoredCallbackTrace().getTimeStamp() );
      
      CPPUNIT_ASSERT( pProcessor5->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor5->getLastStoredCallbackTrace() == referenceTrace );
      CPPUNIT_ASSERT( pProcessor5->getLastStoredCallbackTrace().getTimeStamp() <  pProcessor6->getLastStoredCallbackTrace().getTimeStamp() );
      
      CPPUNIT_ASSERT( pProcessor6->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor6->getLastStoredCallbackTrace() == referenceTrace );
      CPPUNIT_ASSERT( pProcessor6->getLastStoredCallbackTrace().getTimeStamp() <  pProcessor7->getLastStoredCallbackTrace().getTimeStamp() );
      
      CPPUNIT_ASSERT( pProcessor7->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor7->getLastStoredCallbackTrace() == referenceTrace );
      CPPUNIT_ASSERT( pProcessor7->getLastStoredCallbackTrace().getTimeStamp() <  pProcessor8->getLastStoredCallbackTrace().getTimeStamp() );
      
      CPPUNIT_ASSERT( pProcessor8->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor8->getLastStoredCallbackTrace() == referenceTrace );
      CPPUNIT_ASSERT( pProcessor8->getLastStoredCallbackTrace().getTimeStamp() <  pProcessor9->getLastStoredCallbackTrace().getTimeStamp() );
      
      CPPUNIT_ASSERT( pProcessor9->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor9->getLastStoredCallbackTrace() == referenceTrace );
      CPPUNIT_ASSERT( pProcessor9->getLastStoredCallbackTrace().getTimeStamp() <  pProcessor10->getLastStoredCallbackTrace().getTimeStamp() );
      
      CPPUNIT_ASSERT( pProcessor10->getCallbackCount() == 1 );
      CPPUNIT_ASSERT( pProcessor10->getLastStoredCallbackTrace() == referenceTrace );
      
      sipUA.shutdown();
        
      delete pProcessor1;
      delete pProcessor2;
      delete pProcessor3;
      delete pProcessor4;
      delete pProcessor5;
      delete pProcessor6;
      delete pProcessor7;
      delete pProcessor8;
      delete pProcessor9;
      delete pProcessor10;
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipOutputProcessorTest);


