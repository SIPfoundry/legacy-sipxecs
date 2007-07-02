// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <os/OsConfigDb.h>

#include <utl/PluginHooks.h>
#include <os/OsDateTime.h>
#include <net/SipMessage.h>

#include <registry/RegisterPlugin.h>
#include "SipRedirectorTimeOfDay.h"

#define PLUGIN_LIB_DIR TEST_DIR "/../.libs/"

/**
 * Unittest for SipRedirectorTimeOfDayTest
 *
 * @note
 * Important: This test attempts to dynamically load SipRedirectorTimeOfDay shared library, which
 * includes code that references symbols in RedirectPlugin. RedirectPlugin is statically linked
 * into the test executable. "-rdynamic" flag is required to allow symbols in the execubable
 * to be used to resolve references in the shared library
 * Additionally all methods of SipRedirectorTimeOfDay to be called from the test application are
 * required to be declared virtual to allow linker to defer the binding to happen at runtime.
 *
 */
class SipRedirectorTimeOfDayTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipRedirectorTimeOfDayTest);
   CPPUNIT_TEST(testRedirectorTimeOfDay);
   CPPUNIT_TEST_SUITE_END();

public:
   void testRedirectorTimeOfDay()
      {
         PluginHooks redPlugin( "getRedirectPlugin", "SIP_REDIRECT" );

         OsConfigDb configuration;
         configuration.set("SIP_REDIRECT_HOOK_LIBRARY.TIMEOFDAY",
                           PLUGIN_LIB_DIR "libRedirectorTimeOfDay.so" );
         redPlugin.readConfig(configuration);

         PluginIterator getPlugin(redPlugin);
         SipRedirectorTimeOfDay * plugin = (SipRedirectorTimeOfDay*)(getPlugin.next());

         CPPUNIT_ASSERT(plugin);

         const char Time1[] = "0:2759";
         const char Time2[] = "011A:0119";

         UtlString timeString1(Time1);
         UtlString timeString2(Time2);

         CPPUNIT_ASSERT(plugin->isCurrentTimeValid(timeString1));
         CPPUNIT_ASSERT(!plugin->isCurrentTimeValid(timeString2));

         // Calculate minutes since midnight today 
         unsigned long osCurTimeSinceEpoch = OsDateTime::getSecsSinceEpoch();
         int minFromSunday = (osCurTimeSinceEpoch/60 - 4320)%10080;
         int minFromMorning = minFromSunday%1440;

         // The following test creates a set of daily perdiods for each day
         // of the week, such that at least one period includes current time
         int start = ((minFromMorning-60)>0) ? (minFromMorning-60) : 0;
         int end = ((minFromMorning+60)<1440) ? (minFromMorning+60) : 1439;

         std::string str;
         std::ostringstream ostr;
         for (short i = 0 ; i<7 ; i++ )
         {
            if (i>0) ostr << ":";
            int intervalStart = (1440*i) + start;
            int intervalEnd = (1440*i) + end;
            ostr << std::hex << intervalStart << ":" << intervalEnd;
            // If critically close to the end of the day add an extra interval
            // for each morning to accomodate for the case where the actual
            // test takes place the next morning
            if ( end == 1439 )
            {
               intervalStart = (1440*i);
               intervalEnd = (1440*i) + 60;
               ostr  << ":" << std::hex << intervalStart << ":" << intervalEnd;
            }
         }
         ostr << std::ends;
         UtlString longInterval(ostr.str().c_str());

         CPPUNIT_ASSERT(plugin->isCurrentTimeValid(longInterval));

         const char Response[] =
            "SIP/2.0 302 Moved Temporarily\r\n"
            "Via: SIP/2.0/TCP sipx.local:33855;branch=z9hG4bK-10cb6f9378a12d4218e10ef4dc78ea3d\r\n"
            "To: Sip User <sip:sipuser@pingtel.org>;tag=totag\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Contact: <sip:me@127.0.0.2>;q=0.8;sipx-ValidTime=\"0:2760\";sipx-noroute=voicemail\r\n"
            "Contact: <sips:me@127.0.0.3>;sipx-ValidTime=\"01FE:01FE:13B0:13B0\";q=0.8\r\n"
            "Contact: <sip:me@127.0.0.4>;sipx-ValidTime=\"0:InvalidFormat-ShouldbeRemoved\"\r\n"
            "Expires: 1000\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testResponse( Response, strlen( Response ) );

         Url contactUrl("<sip:me@127.0.0.5>");
         contactUrl.setFieldParameter("sipx-ValidTime", longInterval.data());
         testResponse.setContactField(contactUrl.toString().data(), 4);

         UtlString contact;

         CPPUNIT_ASSERT(plugin->processResponse(testResponse));
         CPPUNIT_ASSERT(3==testResponse.getCountHeaderFields(SIP_CONTACT_FIELD));

         testResponse.getContactField(0, contact);
         CPPUNIT_ASSERT(!contact.compareTo("me@127.0.0.1"));
         testResponse.getContactField(1, contact);
         CPPUNIT_ASSERT(!contact.compareTo("<sip:me@127.0.0.2>;q=0.8;sipx-noroute=voicemail"));
    
         testResponse.getContactField(2, contact);
         CPPUNIT_ASSERT(!contact.compareTo("sip:me@127.0.0.5"));
    
         const char ResponseNoMod[] =
            "SIP/2.0 302 Moved Temporarily\r\n"
            "Via: SIP/2.0/TCP sipx.local:33855;branch=z9hG4bK-10cb6f9378a12d4218e10ef4dc78ea3d\r\n"
            "To: Sip User <sip:sipuser@pingtel.org>;tag=totag\r\n"
            "From: Sip Send <sip:sipsend@pingtel.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-ID: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: me@127.0.0.1\r\n"
            "Contact: <me@127.0.0.1>;sip-validTime=\"DoNotRemoveTheHeader\"\r\n"
            "Contact: <me@127.0.0.1;sipx-alidTime=\"0:1:2:3:4:5:6:7:8\">\r\n"
            "Expires: 1000\r\n"
            "Date: Fri, 16 Jul 2004 02:16:15 GMT\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testResponseNoMod1( ResponseNoMod, strlen( Response ) );
         SipMessage testResponseNoMod2( ResponseNoMod, strlen( Response ) );

         CPPUNIT_ASSERT(plugin->processResponse(testResponseNoMod1));

         UtlString msg1, msg2;
         int len1, len2;
         testResponseNoMod1.getBytes(&msg1, &len1);
         testResponseNoMod2.getBytes(&msg2, &len2);

         CPPUNIT_ASSERT(len1==len2);
         CPPUNIT_ASSERT(!msg1.compareTo(msg2, UtlString::matchCase));
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipRedirectorTimeOfDayTest);
