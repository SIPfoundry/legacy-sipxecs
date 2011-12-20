/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.alarm;

import static org.junit.Assert.assertEquals;

import java.io.InputStream;
import java.io.StringWriter;
import java.util.Arrays;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class AlarmConfigurationTest {
    
    @Test
    public void testGenerateAlarms() throws Exception {
        AlarmConfiguration alarmConf = new AlarmConfiguration();
        alarmConf.setVelocityEngine(TestHelper.getVelocityEngine(TestHelper.getSystemEtcDir()));

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
        
        StringWriter actual = new StringWriter(); 
        alarmConf.writeAlarmsXml(actual, Arrays.asList(alarm1, alarm2));

        InputStream expected = getClass().getResourceAsStream("alarms-config-test.xml");
        assertEquals(IOUtils.toString(expected), actual.toString());
    }
}
