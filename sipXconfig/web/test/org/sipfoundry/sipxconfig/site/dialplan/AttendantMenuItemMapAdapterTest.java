/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.Map;
import java.util.TreeMap;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenu;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuItem;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.site.dialplan.AttendantMenuPanel.AttendantMenuItemMapAdapter;

public class AttendantMenuItemMapAdapterTest extends TestCase {

    AttendantMenuItem item1 = new AttendantMenuItem();
    AttendantMenuItem item2 = new AttendantMenuItem();
    AttendantMenuItem item3 = new AttendantMenuItem();

    AttendantMenuItemMapAdapter adapter;

    protected void setUp() {
        // tree map ensure order is preserved and so unittests
        // can make some assumptions so writing is a little easier
        Map<DialPad, AttendantMenuItem> map = new TreeMap<DialPad, AttendantMenuItem>();
        map.put(DialPad.NUM_1, item1);
        map.put(DialPad.NUM_2, item2);
        map.put(DialPad.NUM_3, item3);
        AttendantMenu menu = new AttendantMenu();
        menu.setMenuItems(map);
        adapter = new AttendantMenuItemMapAdapter(menu);
    }

    public void testSetCurrentEntry() {
        adapter.setCurrentDialPadKey(DialPad.NUM_1);
        adapter.setCurrentMenuItemDialPadKeyAssignment(DialPad.NUM_2);
        assertSame(DialPad.NUM_2, adapter.getCurrentMenuItemDialPadKeyAssignment());
        assertSame(DialPad.NUM_2, adapter.getCurrentDialPadKey());
        assertSame(item1, adapter.getCurrentMenuItem());
    }

    public void testSetCurrentMenuItemDialPadKeyAssignmentToSameAssignment() {
        adapter.setCurrentDialPadKey(DialPad.NUM_1);
        adapter.setCurrentMenuItemDialPadKeyAssignment(DialPad.NUM_1);
        assertSame(DialPad.NUM_1, adapter.getCurrentMenuItemDialPadKeyAssignment());
        assertSame(DialPad.NUM_1, adapter.getCurrentDialPadKey());
        assertSame(item1, adapter.getCurrentMenuItem());
    }
}
