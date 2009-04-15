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

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

import org.custommonkey.xmlunit.XMLTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServer;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServerConfiguration;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServerContacts;

public class AlarmServerConfigurationTest extends XMLTestCase {
    public void testGenerateAlarmServer() throws Exception {
        AlarmServerConfiguration alarmServerConf = new AlarmServerConfiguration();
        alarmServerConf.setVelocityEngine(TestHelper.getVelocityEngine());
        alarmServerConf.setTemplate("commserver/alarm-config.vm");
        AlarmServer server = new AlarmServer();
        server.setEmailNotificationEnabled(true);
        AlarmServerContacts contacts = new AlarmServerContacts();
        List<String> addresses = new ArrayList<String>();
        addresses.add("address1@localhost");
        addresses.add("address2@localhost");
        contacts.setAddresses(addresses);

        server.setContacts(contacts);

        alarmServerConf.generate(server, "/usr/local/sipx/var/log/sipxpbx", "post.example.org");

        String generatedXml = AbstractConfigurationFile.getFileContent(alarmServerConf, null);
        InputStream referenceXmlStream = AlarmConfigurationTest.class.getResourceAsStream("alarm-config-test.xml");
        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
    }
}
