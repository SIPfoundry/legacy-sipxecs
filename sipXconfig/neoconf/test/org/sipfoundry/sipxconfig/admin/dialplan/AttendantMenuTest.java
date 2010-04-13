/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Arrays;
import java.util.Map;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.common.DialPad;

public class AttendantMenuTest extends TestCase {

    public void testRemoveMenuItems() {
        AttendantMenu menu = new AttendantMenu();
        menu.addMenuItem(DialPad.NUM_0, AttendantMenuAction.DIAL_BY_NAME);
        menu.addMenuItem(DialPad.NUM_1, AttendantMenuAction.DIAL_BY_NAME);
        menu.addMenuItem(DialPad.NUM_2, AttendantMenuAction.DIAL_BY_NAME);
        Map<DialPad, AttendantMenuItem> menuItems = menu.getMenuItems();
        assertEquals(3, menuItems.size());

        menu.removeMenuItems(Arrays.asList("2", "0"));
        assertEquals(1, menuItems.size());
    }

    public void testGetNextKey() {
        AttendantMenu menu = new AttendantMenu();
        assertSame(DialPad.NUM_0, menu.getNextKey(DialPad.NUM_0, DialPad.POUND));
        menu.addMenuItem(DialPad.NUM_0, AttendantMenuAction.DIAL_BY_NAME);
        menu.addMenuItem(DialPad.NUM_2, AttendantMenuAction.DIAL_BY_NAME);
        assertSame(DialPad.NUM_1, menu.getNextKey(DialPad.NUM_0, DialPad.POUND));
        assertSame(DialPad.NUM_3, menu.getNextKey(DialPad.NUM_2, DialPad.POUND));
    }

    public void testReset() {
        AttendantMenu menu = new AttendantMenu();
        menu.addMenuItem(DialPad.NUM_7, AttendantMenuAction.DIAL_BY_NAME);
        menu.addMenuItem(DialPad.NUM_8, AttendantMenuAction.DIAL_BY_NAME);

        Map<DialPad, AttendantMenuItem> menuItems = menu.getMenuItems();
        menu.reset(false);
        assertEquals(2, menuItems.size());
    }

    public void testResetPermanent() {
        AttendantMenu menu = new AttendantMenu();
        menu.addMenuItem(DialPad.NUM_7, AttendantMenuAction.DIAL_BY_NAME);
        menu.addMenuItem(DialPad.NUM_8, AttendantMenuAction.DIAL_BY_NAME);

        Map<DialPad, AttendantMenuItem> menuItems = menu.getMenuItems();
        menu.reset(true);
        assertEquals(4, menuItems.size());
    }
}
