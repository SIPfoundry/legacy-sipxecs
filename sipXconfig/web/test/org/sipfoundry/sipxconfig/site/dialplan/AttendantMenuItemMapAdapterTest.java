/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
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

    private final AttendantMenuItem item1 = new AttendantMenuItem();
    private final AttendantMenuItem item2 = new AttendantMenuItem();
    private final AttendantMenuItem item3 = new AttendantMenuItem();

    private AttendantMenuItemMapAdapter adapter;
    private final AttendantMenu menu = new AttendantMenu();

    @Override
    protected void setUp() {
        // tree map ensure order is preserved and so unittests
        // can make some assumptions so writing is a little easier
        Map<DialPad, AttendantMenuItem> map = new TreeMap<DialPad, AttendantMenuItem>();
        map.put(DialPad.NUM_1, item1);
        map.put(DialPad.NUM_2, item2);
        map.put(DialPad.NUM_3, item3);
        menu.setMenuItems(map);

        adapter = new AttendantMenuItemMapAdapter(menu);
    }

    public void testSetCurrentEntry() {
        adapter.setCurrentDialPadKey(DialPad.NUM_1);
        adapter.setCurrentMenuItemDialPadKeyAssignment(DialPad.NUM_2);
        assertSame(DialPad.NUM_1, adapter.getCurrentMenuItemDialPadKeyAssignment());
        assertSame(DialPad.NUM_1, adapter.getCurrentDialPadKey());
        assertSame(item1, adapter.getCurrentMenuItem());
    }

    public void testSetCurrentMenuItemDialPadKeyAssignmentToSameAssignment() {
        adapter.setCurrentDialPadKey(DialPad.NUM_1);
        adapter.setCurrentMenuItemDialPadKeyAssignment(DialPad.NUM_1);
        assertSame(DialPad.NUM_1, adapter.getCurrentMenuItemDialPadKeyAssignment());
        assertSame(DialPad.NUM_1, adapter.getCurrentDialPadKey());
        assertSame(item1, adapter.getCurrentMenuItem());
    }

    public void testChangeDialPadKeys() {
        // assign item 1 to dial pad 2
        adapter.setCurrentDialPadKey(DialPad.NUM_1);
        adapter.setCurrentMenuItemDialPadKeyAssignment(DialPad.NUM_2);

        // assign item 2 to dial pad 3
        adapter.setCurrentDialPadKey(DialPad.NUM_2);
        adapter.setCurrentMenuItemDialPadKeyAssignment(DialPad.NUM_3);

        // assign item 3 to dial pad 1
        adapter.setCurrentDialPadKey(DialPad.NUM_3);
        adapter.setCurrentMenuItemDialPadKeyAssignment(DialPad.NUM_1);

        assertSame(item3, menu.getMenuItems().get(DialPad.NUM_1));
        assertSame(item1, menu.getMenuItems().get(DialPad.NUM_2));
        assertSame(item2, menu.getMenuItems().get(DialPad.NUM_3));
    }

    public void testDuplicateDialPadKey() {
        // assign item 1 to dial pad 2
        adapter.setCurrentDialPadKey(DialPad.NUM_1);
        adapter.setCurrentMenuItemDialPadKeyAssignment(DialPad.NUM_2);

        // assign item 2 to dial pad 2
        adapter.setCurrentDialPadKey(DialPad.NUM_2);
        adapter.setCurrentMenuItemDialPadKeyAssignment(DialPad.NUM_2);

        // assign item 3 to dial pad 2
        adapter.setCurrentDialPadKey(DialPad.NUM_3);
        adapter.setCurrentMenuItemDialPadKeyAssignment(DialPad.NUM_2);

        // should have same menu
        assertSame(item1, menu.getMenuItems().get(DialPad.NUM_1));
        assertSame(item2, menu.getMenuItems().get(DialPad.NUM_2));
        assertSame(item3, menu.getMenuItems().get(DialPad.NUM_3));
    }
}
