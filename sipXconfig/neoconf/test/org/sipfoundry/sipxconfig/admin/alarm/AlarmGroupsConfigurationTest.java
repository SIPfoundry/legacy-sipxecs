/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.alarm;

import java.io.InputStream;

import static java.util.Arrays.asList;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile;

public class AlarmGroupsConfigurationTest extends TestCase {
    public void testGenerateAlarmServer() throws Exception {
        AlarmGroupsConfiguration alarmGroupsConf = new AlarmGroupsConfiguration();
        alarmGroupsConf.setVelocityEngine(TestHelper.getVelocityEngine());
        alarmGroupsConf.setTemplate("alarms/alarm-groups.vm");

        AlarmGroup group1 = new AlarmGroup();
        group1.setName("emergency");
        group1.setEmailAddresses(asList("test_email1@example.com"));
        group1.setSmsAddresses(asList("test_sms1@example.com"));

        alarmGroupsConf.generate(asList(group1));

        String generatedXml = AbstractConfigurationFile.getFileContent(alarmGroupsConf, null);
        InputStream referenceXmlStream = getClass().getResourceAsStream("alarm-groups-test.xml");
        assertEquals(IOUtils.toString(referenceXmlStream), generatedXml);
    }
}
