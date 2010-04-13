/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phone;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.acme.AcmePhone;
import org.sipfoundry.sipxconfig.setting.Group;

import junit.framework.TestCase;

import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class LineTest extends TestCase {

    /**
     * This tests that line groups are ignored in favor of using the phone groups instead
     */
    public void testLineGroupsIgnored() {
        Phone phone = new AcmePhone();
        Group phoneGroup = new Group();
        phone.addGroup(phoneGroup);

        Line line = phone.createLine();
        Group lineGroup = new Group();
        line.addGroup(lineGroup);

        assertNotSame(lineGroup, line.getGroups().iterator().next());
    }

    public void testNoUserGetUriAndDisplayLabel() {
        DeviceDefaults defaults = new DeviceDefaults();
        defaults.setDomainManager(TestHelper.getTestDomainManager("sipfoundry.org"));

        PhoneContext phoneContext = createNiceMock(PhoneContext.class);
        phoneContext.getPhoneDefaults();
        expectLastCall().andReturn(defaults).anyTimes();
        replay(phoneContext);

        Phone phone = new AcmePhone();
        PhoneModel model = new PhoneModel("acmePhone");
        model.setModelFilePath("acmePhone");
        phone.setModel(model);
        phone.setPhoneContext(phoneContext);

        phone.setModelFilesContext(TestHelper.getModelFilesContext());
        Line line = phone.createLine();
        line.setModelFilesContext(TestHelper.getModelFilesContext());
        phone.addLine(line);

        LineInfo li = new LineInfo();
        li.setDisplayName("Display Name");
        li.setUserId("user_id");
        li.setRegistrationServer("sipfoundry.org");
        phone.setLineInfo(line, li);

        assertEquals("\"Display Name\"<sip:user_id@sipfoundry.org>", line.getUri());
        assertEquals("user_id", line.getDisplayLabel());

        li.setRegistrationServerPort("5060");
        phone.setLineInfo(line, li);
        assertEquals("\"Display Name\"<sip:user_id@sipfoundry.org>", line.getUri());

        li.setRegistrationServerPort("5070");
        phone.setLineInfo(line, li);
        assertEquals("\"Display Name\"<sip:user_id@sipfoundry.org:5070>", line.getUri());

        verify(phoneContext);
    }

    public void testNoUserGetAddrSpec() {
        DeviceDefaults defaults = new DeviceDefaults();
        defaults.setDomainManager(TestHelper.getTestDomainManager("sipfoundry.org"));

        PhoneContext phoneContext = createNiceMock(PhoneContext.class);
        phoneContext.getPhoneDefaults();
        expectLastCall().andReturn(defaults).anyTimes();
        replay(phoneContext);

        Phone phone = new AcmePhone();
        PhoneModel model = new PhoneModel("acmePhone");
        model.setModelFilePath("acmePhone");
        phone.setModel(model);
        phone.setPhoneContext(phoneContext);

        phone.setModelFilesContext(TestHelper.getModelFilesContext());
        Line line = phone.createLine();
        line.setModelFilesContext(TestHelper.getModelFilesContext());
        phone.addLine(line);

        LineInfo li = new LineInfo();
        li.setDisplayName("Display Name");
        li.setUserId("user_id");
        li.setRegistrationServer("sipfoundry.org");
        phone.setLineInfo(line, li);

        assertEquals("sip:user_id@sipfoundry.org", line.getAddrSpec());

        li.setRegistrationServerPort("5060");
        phone.setLineInfo(line, li);
        assertEquals("sip:user_id@sipfoundry.org", line.getAddrSpec());

        li.setRegistrationServerPort("5070");
        phone.setLineInfo(line, li);
        assertEquals("sip:user_id@sipfoundry.org:5070", line.getAddrSpec());

        verify(phoneContext);
    }

    public void testGetDisplayLabelViaUser() {
        Line line = new Line();
        User u = new User();
        u.setUserName("joe");
        line.setUser(u);
        assertEquals("joe", line.getDisplayLabel());
    }

    public void testNoUserGetAuthorizationName() {
        PhoneModel model = new PhoneModel("acmePhone");
        model.setModelFilePath("acmePhone");

        Phone phone = new AcmePhone();
        phone.setModelFilesContext(TestHelper.getModelFilesContext());
        phone.setModel(model);
        phone.setSerialNumber("0000DEADBEEF");

        Line line = phone.createLine();
        line.setModelFilesContext(TestHelper.getModelFilesContext());
        phone.addLine(line);

        assertNull(line.getAuthenticationUserName());
    }

    public void testUserGetAuthorizationName() {
        PhoneModel model = new PhoneModel("acmePhone");
        model.setModelFilePath("acmePhone");

        Phone phone = new AcmePhone();
        phone.setModelFilesContext(TestHelper.getModelFilesContext());
        phone.setModel(model);
        phone.setSerialNumber("0000DEADBEEF");

        Line line = phone.createLine();
        line.setModelFilesContext(TestHelper.getModelFilesContext());
        phone.addLine(line);

        User u = new User();
        u.setUserName("user_id");
        line.setUser(u);

        assertEquals("user_id/0000DEADBEEF", line.getAuthenticationUserName());
    }
}
