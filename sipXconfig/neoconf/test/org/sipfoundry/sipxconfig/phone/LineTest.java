/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.acme.AcmePhone;
import org.sipfoundry.sipxconfig.setting.Group;

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

        IMocksControl phoneContextCtrl = EasyMock.createNiceControl();
        PhoneContext phoneContext = phoneContextCtrl.createMock(PhoneContext.class);
        phoneContext.getPhoneDefaults();
        phoneContextCtrl.andReturn(defaults).anyTimes();
        phoneContextCtrl.replay();        

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

        String actual = line.getUri();
        assertEquals("\"Display Name\"<sip:user_id@sipfoundry.org>", actual);
        assertEquals("user_id", line.getDisplayLabel());

        phoneContextCtrl.verify();
    }

    public void testGetDisplayLabelViaUser() {
        Line line = new Line();
        User u = new User();
        u.setUserName("joe");
        line.setUser(u);
        assertEquals("joe", line.getDisplayLabel());
    }
}
