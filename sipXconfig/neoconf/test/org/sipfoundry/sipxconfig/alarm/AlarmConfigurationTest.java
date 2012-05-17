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
import java.text.MessageFormat;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Locale;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.mail.MailManager;
import org.springframework.context.support.AbstractMessageSource;

public class AlarmConfigurationTest {
    
    @Test
    public void testGenerateAlarmServer() throws Exception {        
        AlarmConfiguration alarmServerConf = new AlarmConfiguration();
        AlarmServer server = new AlarmServer();
        server.setAlarmNotificationEnabled(true);
        String host = "post.example.org";
        StringWriter actual = new StringWriter();
        AlarmGroup g1 = new AlarmGroup();
        g1.setName("g1");
        g1.setEmailAddresses(Arrays.asList("e1@example.org", "e2@example.org"));
        AlarmGroup g2 = new AlarmGroup();        
        g2.setName("g2");
        List<AlarmGroup> groups = Arrays.asList(g1, g2);
        Alarm a1 = new Alarm(new AlarmDefinition("a1"));
        Alarm a2 = new Alarm(new AlarmDefinition("a2"));
        a1.setGroupName(g1.getName());
        a2.setGroupName(g2.getName());
        List<Alarm> alarms = Arrays.asList(a1, a2);
        Address smtp = new Address(MailManager.SMTP, "mail.example.org");
        alarmServerConf.writeEmailHandlerConfig(actual, alarms, groups, server, host, smtp);
        InputStream expected = getClass().getResourceAsStream("expected-sipxtrap-handler.yaml");
        assertEquals(IOUtils.toString(expected), actual.toString());
    }
    
    @Test
    public void testAlarmYml() throws Exception {
        StringWriter actual = new StringWriter();
        AlarmConfiguration c = new AlarmConfiguration();
        c.setMessageSource(new AbstractMessageSource() {
            protected MessageFormat resolveCode(String arg0, Locale arg1) {
                return new MessageFormat(arg0);
            }
        });
        List<Alarm> alarms = Arrays.asList(new Alarm(AdminContext.ALARM_LOGIN_FAILED));
        c.writeAlarms(actual, alarms, Locale.ENGLISH);
        InputStream expected = getClass().getResourceAsStream("expected-alarms.yaml");
        assertEquals(IOUtils.toString(expected), actual.toString());
    }
}
