
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

#include <errno.h>
#include <limits.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <fstream>
#include <stdio.h>
#include "os/OsResourceLimit.h"

typedef OsResourceLimit::Limit Limit;

#define umaxof(t) (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | \
                    (0xFULL << ((sizeof(t) * 8ULL) - 4ULL)))

#define smaxof(t) (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | \
                    (0x7ULL << ((sizeof(t) * 8ULL) - 4ULL)))

class OsResourceLimitTest : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(OsResourceLimitTest);
  CPPUNIT_TEST(testResourceLimit);
  CPPUNIT_TEST(testResourceLimitFromFile);
  CPPUNIT_TEST_SUITE_END();
public:


  void testResourceLimit()
  {
    //
    // Test negative absolute value
    //
    Limit testNegative = -1024;
    Limit testNegativeOffset = (umaxof(Limit) - 1024) + 1;

    CPPUNIT_ASSERT(testNegative == testNegativeOffset);

    OsResourceLimit resource;

    Limit rescur = 0;
    Limit newresCur = 0;
    Limit resmax = 0;
    Limit resmaxNew = 0;
    //
    // Raise the current file handle limit to maximum allowable
    //
    CPPUNIT_ASSERT(resource.getFileDescriptorLimit(rescur, resmax));
    CPPUNIT_ASSERT(resource.setFileDescriptorLimit(resmax));
    CPPUNIT_ASSERT(resource.getFileDescriptorLimit(newresCur, resmax));
    CPPUNIT_ASSERT(newresCur == resmax);
    CPPUNIT_ASSERT(resource.setFileDescriptorLimit(rescur, resmax));

    //
    // Raise the current file handle limit more than the maximum allowable
    //
    CPPUNIT_ASSERT(resource.getFileDescriptorLimit(rescur, resmax));
    resmaxNew = resmax + 2;
    if (resmaxNew > resmax)
    {
      //
      // Only verify if resmax can still be increases (less than INT_MAX)
      //
      CPPUNIT_ASSERT(!resource.setFileDescriptorLimit(resmaxNew));
      CPPUNIT_ASSERT(resource.setFileDescriptorLimit(rescur, resmax));
    }
    

    //
    // Raise the current file handle limit more than the maximum allowable
    //
    CPPUNIT_ASSERT(resource.getFileDescriptorLimit(rescur, resmax));
    if (resmax < umaxof(Limit))
    {
      CPPUNIT_ASSERT(!resource.setFileDescriptorLimit(umaxof(Limit)));
      CPPUNIT_ASSERT(resource.setFileDescriptorLimit(rescur, resmax));
    }

    //
    // Test negative values.  This should succeed because rlimit_t is
    // an unsigned int and therefore will assume the offset value
    // of the negative number
    //
    if (testNegativeOffset < resmax)
    {
      CPPUNIT_ASSERT(!resource.setFileDescriptorLimit(-1024));
      CPPUNIT_ASSERT(resource.setFileDescriptorLimit(rescur, resmax));
    }


    //
    // Raise the current core file size limit to maximum allowable
    //
    CPPUNIT_ASSERT(resource.getCoreFileLimit(rescur, resmax));
    CPPUNIT_ASSERT(resource.setCoreFileLimit(resmax));
    CPPUNIT_ASSERT(resource.getCoreFileLimit(newresCur, resmax));
    CPPUNIT_ASSERT(newresCur == resmax);
    CPPUNIT_ASSERT(resource.setCoreFileLimit(rescur, resmax));

    //
    // Raise the current core file limit more than the maximum allowable
    //
    CPPUNIT_ASSERT(resource.getCoreFileLimit(rescur, resmax));
    if (resmax < umaxof(Limit))
    {
      //
      // Only verify if resmax can still be increased (less than umaxof(Limit))
      //
      CPPUNIT_ASSERT(!resource.setCoreFileLimit(umaxof(Limit)));
      CPPUNIT_ASSERT(resource.setCoreFileLimit(rescur, resmax));
    }

    //
    // Test negative values.  This should succeed because rlimit_t is
    // an unsigned int and therefore will assume the offset value
    // of the negative number
    //
    if (testNegativeOffset < resmax)
    {
      CPPUNIT_ASSERT(resource.setCoreFileLimit(-1024));
      CPPUNIT_ASSERT(resource.setCoreFileLimit(rescur, resmax));
    }
  }

  void testResourceLimitFromFile()
  {
    OsResourceLimit resource;

    Limit rescur = 0;
    Limit resmax = 0;
    CPPUNIT_ASSERT(resource.getFileDescriptorLimit(rescur, resmax));

    std::string executableName = "sipxproxy";
    std::string configFile = "ResourceLimit.ini";

    std::string fdCurName = executableName + RLIM_FD_CUR_SUFFIX;
    std::string fdMaxName = executableName + RLIM_FD_MAX_SUFFIX;

    std::string coreEnabled = executableName + RLIM_CORE_ENABLED_SUFFIX;
    std::string coreCurName = executableName + RLIM_CORE_CUR_SUFFIX;
    std::string coreMaxName = executableName + RLIM_CORE_MAX_SUFFIX;

    //
    // Test normal
    //
    {
      remove(configFile.c_str());
      std::ofstream config(configFile.c_str());
      config << fdCurName << " = " << resmax / 2 << std::endl;
      config << fdMaxName << " = " << resmax << std::endl;
      CPPUNIT_ASSERT(resource.getCoreFileLimit(rescur, resmax));
      config << coreEnabled << " = " << "true" << std::endl;
      config << coreCurName << " = " << resmax / 2 << std::endl;
      config << coreMaxName << " = " << resmax << std::endl;
    }

    CPPUNIT_ASSERT(resource.setApplicationLimits(executableName, configFile));

    Limit newFdLimit = 0;
    Limit newCoreLimit = 0;

    CPPUNIT_ASSERT(resource.getFileDescriptorLimit(newFdLimit, resmax));
    CPPUNIT_ASSERT(newFdLimit == resmax / 2);
    CPPUNIT_ASSERT(resource.getCoreFileLimit(newCoreLimit, resmax));
    CPPUNIT_ASSERT(newCoreLimit == resmax / 2);

    //
    // Test core disabled
    //
    {
      remove(configFile.c_str());
      std::ofstream config(configFile.c_str());
      CPPUNIT_ASSERT(resource.getFileDescriptorLimit(newFdLimit, resmax));
      config << fdCurName << " = " << resmax / 2 << std::endl;
      config << fdMaxName << " = " << resmax << std::endl;
      CPPUNIT_ASSERT(resource.getCoreFileLimit(rescur, resmax));
      config << coreEnabled << " = " << "false" << std::endl;
      config << coreCurName << " = " << resmax / 2 << std::endl;
      config << coreMaxName << " = " << resmax << std::endl;
    }
    
    CPPUNIT_ASSERT(resource.setApplicationLimits(executableName, configFile));
    
    newFdLimit = 0;
    newCoreLimit = 0;

    CPPUNIT_ASSERT(resource.getFileDescriptorLimit(newFdLimit, resmax));
    CPPUNIT_ASSERT(newFdLimit == resmax / 2);
    CPPUNIT_ASSERT(resource.getCoreFileLimit(newCoreLimit, resmax));
    CPPUNIT_ASSERT(newCoreLimit == resmax / 2);

    //
    // Test core limits unspecified
    //
    {
      remove(configFile.c_str());
      std::ofstream config(configFile.c_str());
      CPPUNIT_ASSERT(resource.getFileDescriptorLimit(newFdLimit, resmax));
      config << fdCurName << " = " << resmax / 2 << std::endl;
      config << fdMaxName << " = " << resmax << std::endl;
      config << coreEnabled << " = " << "true" << std::endl;
    }

    CPPUNIT_ASSERT(resource.setApplicationLimits(executableName, configFile));

    newFdLimit = 0;
    newCoreLimit = 0;

    CPPUNIT_ASSERT(resource.getFileDescriptorLimit(newFdLimit, resmax));
    CPPUNIT_ASSERT(newFdLimit == resmax / 2);
    CPPUNIT_ASSERT(resource.getCoreFileLimit(newCoreLimit, resmax));
    CPPUNIT_ASSERT(newCoreLimit == resmax);
    

    //
    // Negative test
    //
    CPPUNIT_ASSERT(!resource.setApplicationLimits(executableName, "bogus"));
    CPPUNIT_ASSERT(!resource.setApplicationLimits("bogus", configFile));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsResourceLimitTest);