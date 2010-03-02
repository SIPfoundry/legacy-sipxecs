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

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.service.SipxAlarmService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.SipxServiceTestBase;

public class AlarmServerConfigurationTest extends SipxServiceTestBase {
    public void testGenerateAlarmServer() throws Exception {
        AlarmServerConfiguration alarmServerConf = new AlarmServerConfiguration();
        alarmServerConf.setVelocityEngine(TestHelper.getVelocityEngine());
        alarmServerConf.setTemplate("commserver/alarm-config.vm");

        SipxAlarmService alarmService = new SipxAlarmService();
        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxAlarmService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(alarmService).anyTimes();
        EasyMock.replay(sipxServiceManager);
        alarmServerConf.setSipxServiceManager(sipxServiceManager);

        AlarmServer server = new AlarmServer();
        server.setAlarmNotificationEnabled(true);

        AlarmServerManager alarmServerManager = EasyMock.createMock(AlarmServerManager.class);
        alarmServerManager.getAlarmServer();
        EasyMock.expectLastCall().andReturn(server).anyTimes();
        alarmServerManager.getHost();
        EasyMock.expectLastCall().andReturn("post.example.org").anyTimes();
        alarmServerManager.getLogDirectory();
        EasyMock.expectLastCall().andReturn("/usr/local/sipx/var/log/sipxpbx").anyTimes();
        EasyMock.replay(alarmServerManager);
        alarmServerConf.setAlarmServerManager(alarmServerManager);

        assertCorrectFileGeneration(alarmServerConf, "alarm-config-test.xml");
    }
}
