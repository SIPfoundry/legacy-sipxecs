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

import java.util.Collection;
import java.util.Map;
import java.util.TreeMap;

import org.sipfoundry.sipxconfig.common.DialPad;

public class AttendantMenu {

    private Map<DialPad, AttendantMenuItem> m_menuItems = new TreeMap<DialPad, AttendantMenuItem>();

    public void setMenuItems(Map<DialPad, AttendantMenuItem> menuItems) {
        m_menuItems = menuItems;
    }

    public Map<DialPad, AttendantMenuItem> getMenuItems() {
        return m_menuItems;
    }

    private void addMenuItem(DialPad key, AttendantMenuItem menuItem) {
        m_menuItems.put(key, menuItem);
    }

    public void addMenuItem(DialPad key, AttendantMenuAction action, String parameter) {
        addMenuItem(key, new AttendantMenuItem(action, parameter));
    }

    public void addMenuItem(DialPad key, AttendantMenuAction action) {
        addMenuItem(key, new AttendantMenuItem(action));
    }

    public void removeMenuItem(DialPad key) {
        if (m_menuItems == null) {
            return;
        }

        m_menuItems.remove(key);
    }

    public void reset(boolean permanent) {
        m_menuItems.clear();
        addMenuItem(DialPad.NUM_0, new AttendantMenuItem(AttendantMenuAction.OPERATOR));
        addMenuItem(DialPad.STAR, new AttendantMenuItem(AttendantMenuAction.REPEAT_PROMPT));
        if (permanent) {
            addMenuItem(DialPad.NUM_9, new AttendantMenuItem(AttendantMenuAction.DIAL_BY_NAME));
            addMenuItem(DialPad.POUND, new AttendantMenuItem(AttendantMenuAction.VOICEMAIL_LOGIN));
        }
    }

    public void removeMenuItems(Collection<String> items) {
        for (String name : items) {
            m_menuItems.remove(DialPad.getByName(name));
        }
    }

    public DialPad getNextKey(DialPad minKey, DialPad maxKey) {
        DialPad[] range = DialPad.getRange(minKey, maxKey);
        for (DialPad key : range) {
            if (!m_menuItems.containsKey(key)) {
                return key;
            }
        }
        return maxKey;
    }
}
