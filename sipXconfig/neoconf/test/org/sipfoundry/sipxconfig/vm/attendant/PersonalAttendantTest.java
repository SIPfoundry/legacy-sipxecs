/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.vm.attendant;

import java.io.InputStream;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenu;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuAction;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;

public class PersonalAttendantTest extends TestCase {

    private MemoryProfileLocation m_location;
    private VelocityProfileGenerator m_profileGenerator;

    protected void setUp() throws Exception {
        m_location = new MemoryProfileLocation();
        m_profileGenerator = TestHelper.getProfileGenerator();
    }

    public void testGenerateUserProfile() throws Exception {
        PersonalAttendant personalAttendant = new PersonalAttendant();
        personalAttendant.setUser(new User());
        personalAttendant.setOperator("sip:123@example.org");

        AttendantMenu menu = new AttendantMenu();
        menu.addMenuItem(DialPad.NUM_1, AttendantMenuAction.GOTO_EXTENSION, "sip:201@example.org");
        menu.addMenuItem(DialPad.NUM_2, AttendantMenuAction.GOTO_EXTENSION, "sip:202@example.com");
        personalAttendant.setMenu(menu);

        personalAttendant.generateProfile(m_location, m_profileGenerator);

        InputStream expected = getClass().getResourceAsStream("savemessage.user.vxml");

        assertEquals(IOUtils.toString(expected), m_location.toString());
        expected.close();
    }

    public void testGenerateGenericProfile() throws Exception {
        PersonalAttendant personalAttendant = new PersonalAttendant();

        personalAttendant.generateProfile(m_location, m_profileGenerator);

        InputStream expected = getClass().getResourceAsStream("savemessage.generic.vxml");

        assertEquals(IOUtils.toString(expected), m_location.toString());
        expected.close();
    }
}
