//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include <limits.h>

#include <sipxunit/TestUtilities.h>

#include "../AlarmData.h"
#include "../AlarmServer.h"
#include "alarm/Alarm.h"
#include "testlib/FileTestContext.h"

typedef struct
{
   const char* id;
   const char* code;
   const char* component;
   const OsSysLogPriority severity;
   const char* description;
   const char* resolution;
   const bool actions[cAlarmData::eActionMax];
   const int max_report;
   const int min_threshold;
} alarmRowData;

class AlarmLocalizationTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(AlarmLocalizationTest);
   CPPUNIT_TEST(testLocalizedAlarms);
   CPPUNIT_TEST(testParameterSubstitution);
   CPPUNIT_TEST_SUITE_END();

public:

   FileTestContext* mAlarmTestContext;
   FileTestContext* mAlarmTestContext2;
   FileTestContext* mAlarmTestContext3;
   FileTestContext* mAlarmTestContext4;

   void setUp()
   {
      OsSysLog::initialize(0, "alarm");
      OsSysLog::setOutputFile(0, "alarmTest.log");
      OsSysLog::setLoggingPriority(PRI_DEBUG);

      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG, "AlarmLocalizationTest::setUp");

      // set up parallel directory structure to match actual
      mAlarmTestContext = new FileTestContext(TEST_DATA_DIR "/alarm-localization",
         TEST_WORK_DIR "/alarms-localization-config");
      mAlarmTestContext2 = new FileTestContext(TEST_DATA_DIR "/alarm-localization",
         TEST_WORK_DIR "/alarms-localization-config/alarms");
      mAlarmTestContext3 = new FileTestContext(TEST_DATA_DIR "/alarm-localization",
         TEST_WORK_DIR "/alarms-localization-share");
      mAlarmTestContext4 = new FileTestContext(TEST_DATA_DIR "/alarm-localization",
         TEST_WORK_DIR "/alarms-localization-share/alarms");

      // copy test files into parallel structure
      mAlarmTestContext->inputFile("alarm-config.xml");
      mAlarmTestContext->inputFile("alarm-groups.xml");
      mAlarmTestContext2->inputFile("test-localization-config.xml");
      mAlarmTestContext4->inputFile("test-localization-strings.xml");
      mAlarmTestContext4->inputFile("test-localization-strings_fr.xml");

      // tell SipXecsService to use the parallel structure
      mAlarmTestContext->setSipxDir(SipXecsService::ConfigurationDirType);
      mAlarmTestContext3->setSipxDir(SipXecsService::DataDirType);

      // now load the test files
      cAlarmServer::getInstance()->init();
   }

   void tearDown()
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG, "AlarmLocalizationTest::tearDown");
      cAlarmServer::getInstance()->cleanup();
      delete mAlarmTestContext;
      delete mAlarmTestContext2;
      delete mAlarmTestContext3;
      delete mAlarmTestContext4;
      OsSysLog::flush();
      fflush(stdout);
   }

   void testLocalizedAlarms()
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG, "AlarmLocalizationTest::testLoadAlarms");

      alarmRowData expectedResult[] =
      {
         {
            "TEST_LOG",
            "SCF00011",
            "alarmTest",
            PRI_ALERT,
            "C'est une épreuve de la système alarme. La seule parameter devrait être ici: {0}, et c'est tout.",
            "C'est aussi une épreuve de l'insertion des parametres, des accents, et des virgules.",
            { true, true, true, true },
            3,
            0
         },
         {
            "PARAMETER_SUBSTITUTION",
            "SCF23089",
            "sipXsupervisor",
            PRI_WARNING,
            "La deuxième parametre est {1}, puis parametre {0}",
            "Check for bugs in assembleMsg!",
            { true, true, true, true },
            INT_MAX,
            0
         },
         {
            "SPECIAL_CHARACTERS",
            "SCF39289",
            "sipXsupervisor",
            PRI_DEBUG,
            "Print special characters such as <, >, and & properly.",
            "Text must be escaped in the strings file, and unescaped before sending notifications.",
            { true, true, true, true },
            INT_MAX,
            0
         },
         {
            "MISSING_TRANSLATION",
            "SCF24189",
            "sipXsupervisor",
            PRI_WARNING,
            "This is missing from the _fr strings file.",
            "It should be displayed in English.",
            { true, true, true, true },
            INT_MAX,
            0
         }
      };

      cAlarmData* alarmData;
      for (size_t i=0; i<sizeof(expectedResult)/sizeof(alarmRowData); i++)
      {
         char msg[1000];
         UtlString tempStr(expectedResult[i].id);
         alarmData = cAlarmServer::getInstance()->lookupAlarm(tempStr);
         sprintf(msg, "in definition of alarm %s", expectedResult[i].id);
         CPPUNIT_ASSERT_MESSAGE(msg, alarmData!=0);
         ASSERT_STR_EQUAL_MESSAGE(msg, expectedResult[i].code, alarmData->getCode().data());
         CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, (int)expectedResult[i].severity, (int)alarmData->getSeverity());
         ASSERT_STR_EQUAL_MESSAGE(msg, expectedResult[i].description, alarmData->getDescription().data());
         ASSERT_STR_EQUAL_MESSAGE(msg, expectedResult[i].resolution, alarmData->getResolution().data());
         for (int j=0; j<cAlarmData::eActionMax; j++)
         {
            sprintf(msg, "alarm %s, action %d", expectedResult[i].id, j);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, (int)expectedResult[i].actions[(int)j], (int)alarmData->getAction((cAlarmData::eAlarmActions)j));
         }
         //CPPUNIT_ASSERT_EQUAL(expectedResult[i].min_threshold, alarmData.getMinThreshold());
      }

   }

   void testParameterSubstitution()
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG, "AlarmLocalizationTest::testParameterSubstitution");
      UtlString localhost("localhost");
      UtlString alarmId("PARAMETER_SUBSTITUTION");
      UtlSList alarmParams;
      UtlString alarmParam("1");
      alarmParams.append(&alarmParam);
      UtlString alarmParam2 = "2";
      alarmParams.append(&alarmParam2);
      cAlarmServer::getInstance()->handleAlarm(localhost, alarmId, alarmParams);
      // check for "La deuxième parametre est 2, puis parametre 1" in log
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(AlarmLocalizationTest);
