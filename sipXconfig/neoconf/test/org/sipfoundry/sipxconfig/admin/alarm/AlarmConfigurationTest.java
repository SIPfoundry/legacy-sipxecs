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
import java.util.Arrays;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile;

public class AlarmConfigurationTest extends TestCase {
    public void testGenerateAlarms() throws Exception {
        AlarmConfiguration alarmConf = new AlarmConfiguration();
        alarmConf.setVelocityEngine(TestHelper.getVelocityEngine());
        alarmConf.setTemplate("alarms/sipXalarms-config.vm");

        Alarm alarm1 = new Alarm();
        alarm1.setAlarmId("LOGIN_FAILED");
        alarm1.setCode("SPX00005");
        alarm1.setSeverity("warning");
        alarm1.setComponent("sipXconfig");
        alarm1.setGroupName("default");
        alarm1.setMinThreshold(3);

        Alarm alarm2 = new Alarm();
        alarm2.setAlarmId("PROCESS_FAILED");
        alarm2.setCode("SPX00004");
        alarm2.setSeverity("crit");
        alarm2.setComponent("sipXsupervisor");
        alarm2.setGroupName("disabled");

        alarmConf.generate(Arrays.asList(alarm1, alarm2));

        String generatedXml = AbstractConfigurationFile.getFileContent(alarmConf, null);

        InputStream referenceXmlStream = getClass().getResourceAsStream("alarms-config-test.xml");
        assertEquals(IOUtils.toString(referenceXmlStream), generatedXml);
    }
}
