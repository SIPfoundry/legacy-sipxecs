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

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile;

public class AlarmServerConfigurationTest extends TestCase {
    public void testGenerateAlarmServer() throws Exception {
        AlarmServerConfiguration alarmServerConf = new AlarmServerConfiguration();
        alarmServerConf.setVelocityEngine(TestHelper.getVelocityEngine());
        alarmServerConf.setTemplate("commserver/alarm-config.vm");
        AlarmServer server = new AlarmServer();
        server.setEmailNotificationEnabled(true);

        alarmServerConf.generate(server, "/usr/local/sipx/var/log/sipxpbx", "post.example.org");

        String generatedXml = AbstractConfigurationFile.getFileContent(alarmServerConf, null);
        InputStream referenceXmlStream = AlarmConfigurationTest.class.getResourceAsStream("alarm-config-test.xml");
        assertEquals(IOUtils.toString(referenceXmlStream), generatedXml);
    }
}
