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
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenu;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuAction;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant.AttendantProfileContext;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant.MenuItem;

public class PersonalAttendantTest extends TestCase {

    private VelocityProfileGenerator m_profileGenerator;

    @Override
    protected void setUp() throws Exception {
        m_profileGenerator = TestHelper.getProfileGenerator();
    }

    public void testGenerateUserProfile() throws Exception {
        PersonalAttendant personalAttendant = new PersonalAttendant();
        personalAttendant.setUser(new User());
        personalAttendant.setOperator("123");

        AttendantMenu menu = new AttendantMenu();
        menu.addMenuItem(DialPad.NUM_1, AttendantMenuAction.TRANSFER_OUT, "201");
        menu.addMenuItem(DialPad.NUM_2, AttendantMenuAction.TRANSFER_OUT, "sip:202@example.com");
        personalAttendant.setMenu(menu);

        MemoryProfileLocation location = new MemoryProfileLocation();
        personalAttendant.generatePropertiesProfile(location, "example.org", m_profileGenerator);

        InputStream expected = getClass().getResourceAsStream("PersonalAttendant.user.properties");

        assertEquals(IOUtils.toString(expected), location.toString());
        expected.close();
    }

    public void testGenerateUserWithNoOperatorProfile() throws Exception {
        PersonalAttendant personalAttendant = new PersonalAttendant();
        personalAttendant.setUser(new User());
        personalAttendant.setOperator(null);

        AttendantMenu menu = new AttendantMenu();
        menu.addMenuItem(DialPad.NUM_1, AttendantMenuAction.TRANSFER_OUT, "201");
        menu.addMenuItem(DialPad.NUM_2, AttendantMenuAction.TRANSFER_OUT, "sip:202@example.com");
        personalAttendant.setMenu(menu);

        MemoryProfileLocation location = new MemoryProfileLocation();
        personalAttendant.generatePropertiesProfile(location, "example.org", m_profileGenerator);

        InputStream expected = getClass().getResourceAsStream("PersonalAttendant.user-no-operator.properties");

        assertEquals(IOUtils.toString(expected), location.toString());
        expected.close();
    }

    public void testGenerateUserProfileWithOverrideLanguage() throws Exception {
        PersonalAttendant personalAttendant = new PersonalAttendant();
        personalAttendant.setUser(new User());
        personalAttendant.setOperator("123");
        personalAttendant.setOverrideLanguage(true);
        personalAttendant.setLanguage("en");

        AttendantMenu menu = new AttendantMenu();
        menu.addMenuItem(DialPad.NUM_1, AttendantMenuAction.TRANSFER_OUT, "201");
        menu.addMenuItem(DialPad.NUM_2, AttendantMenuAction.TRANSFER_OUT, "sip:202@example.com");
        personalAttendant.setMenu(menu);

        MemoryProfileLocation location = new MemoryProfileLocation();
        personalAttendant.generatePropertiesProfile(location, "example.org", m_profileGenerator);

        InputStream expected = getClass().getResourceAsStream("PersonalAttendant.user.overridelanguage.properties");

        assertEquals(IOUtils.toString(expected), location.toString());
        expected.close();
    }

    public void testContext() {
        PersonalAttendant pa = new PersonalAttendant();
        pa.setUser(new User());
        pa.setOperator("123");

        AttendantMenu menu = new AttendantMenu();
        menu.addMenuItem(DialPad.NUM_1, AttendantMenuAction.TRANSFER_OUT, "201");
        menu.addMenuItem(DialPad.NUM_2, AttendantMenuAction.TRANSFER_OUT, "sip:202@example.com");
        pa.setMenu(menu);

        AttendantProfileContext ctx = new PersonalAttendant.AttendantProfileContext(pa, "example.org", null);
        Map<String, Object> map = ctx.getContext();

        List<PersonalAttendant.MenuItem> items = (List<MenuItem>) map.get("menu");

        assertEquals("1", items.get(0).getKey());
        assertEquals("sip:201@example.org", items.get(0).getUri());
        assertEquals("2", items.get(1).getKey());
        assertEquals("sip:202@example.com", items.get(1).getUri());

        assertEquals(2, items.size());

        assertEquals("sip:123@example.org", map.get("operator"));
    }
}
