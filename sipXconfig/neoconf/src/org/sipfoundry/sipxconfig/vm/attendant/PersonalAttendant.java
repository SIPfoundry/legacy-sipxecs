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

import java.util.Map;
import java.util.TreeMap;

import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuItem;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.ProfileLocation;

public class PersonalAttendant extends BeanWithId {
    private User m_user;

    private Map<DialPad, AttendantMenuItem> m_menuItems;

    private String m_operator;

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public String getOperator() {
        return m_operator;
    }

    public void setOperator(String operator) {
        m_operator = operator;
    }

    public void setMenuItems(Map<DialPad, AttendantMenuItem> menuItems) {
        m_menuItems = menuItems;
    }

    public Map<DialPad, AttendantMenuItem> getMenuItems() {
        return m_menuItems;
    }

    public void addMenuItem(DialPad key, AttendantMenuItem menuItem) {
        if (m_menuItems == null) {
            m_menuItems = new TreeMap();
        }

        m_menuItems.put(key, menuItem);
    }

    public void removeMenuItem(DialPad key) {
        if (m_menuItems == null) {
            return;
        }

        m_menuItems.remove(key);
    }
    
    /**
     * Generate personal AA VXML file (from the template)
     * 
     * @param location profile destination
     * @param generator Velocity based VXML generator
     */
    public void generateProfile(ProfileLocation location, ProfileGenerator generator) {
        AttendantProfileContext context = new AttendantProfileContext(this);
        generator.generate(location, context, null, "savemessage.user.vxml");
    }

    public static class AttendantProfileContext extends ProfileContext {
        private PersonalAttendant m_aa;

        public AttendantProfileContext(PersonalAttendant aa) {
            super(null, "sipxvxml/savemessage.vxml.vm");
            m_aa = aa;
        }

        public Map<String, Object> getContext() {
            Map<String, Object> context = super.getContext();
            context.put("aa", m_aa);
            context.put("operator", m_aa.getOperator());
            return context;
        }
    }
}
