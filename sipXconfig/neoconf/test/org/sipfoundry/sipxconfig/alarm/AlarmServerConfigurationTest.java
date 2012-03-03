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
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class AlarmServerConfigurationTest {
    
    @Test
    public void testGenerateAlarmServer() throws Exception {        
        AlarmConfiguration alarmServerConf = new AlarmConfiguration();
        alarmServerConf.setVelocityEngine(TestHelper.getVelocityEngine());
        AlarmServer server = new AlarmServer();
        server.setAlarmNotificationEnabled(true);
        String host = "post.example.org";
        StringWriter actual = new StringWriter();
        alarmServerConf.writeAlarmConfigXml(actual, server, host);       
        InputStream expected = getClass().getResourceAsStream("alarm-config-test.xml");
        assertEquals(IOUtils.toString(expected), actual.toString());
    }
    
    @Test
    public void testAlarmYml() throws Exception {
        StringWriter actual = new StringWriter();
        AlarmConfiguration c = new AlarmConfiguration();
        List<Alarm> alarms = Arrays.asList(new Alarm(AdminContext.ALARM_LOGIN_FAILED));
        c.writeAlarms(actual, alarms);
        assertEquals("LOGIN_FAILED:\n   :groupName: disabled\n   :minThreshold: 3\n", actual.toString());
    }
}
