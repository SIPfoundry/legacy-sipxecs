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

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenu;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuItem;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.ProfileLocation;

public class PersonalAttendant extends BeanWithId {
    private User m_user;

    private AttendantMenu m_menu = new AttendantMenu();

    private String m_operator;

    private String m_language;

    private boolean m_overrideLanguage;

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public boolean getOverrideLanguage() {
        return m_overrideLanguage;
    }

    public void setOverrideLanguage(boolean overrideLanguage) {
        m_overrideLanguage = overrideLanguage;
    }

    public String getLanguage() {
        return m_language;
    }

    public void setLanguage(String language) {
        m_language = language;
    }

    public String getOperator() {
        return m_operator;
    }

    public void setOperator(String operator) {
        m_operator = operator;
    }

    public void setMenu(AttendantMenu menu) {
        m_menu = menu;
    }

    public AttendantMenu getMenu() {
        return m_menu;
    }

    /**
     * Generate personal AA Properties file (from the template)
     *
     * @param location profile destination
     * @param domain SIP domain for which we are generating this file
     * @param generator Velocity based generator
     */
    public void generatePropertiesProfile(ProfileLocation location, String domain, ProfileGenerator generator) {
        // Generate the properties for the new vm
        AttendantProfileContext propertiesContext = new AttendantProfileContext(this, domain,
                "sipxivr/PersonalAttendant.properties.vm");
        generator.generate(location, propertiesContext, null, "PersonalAttendant.properties");
    }

    public static class MenuItem {
        private final String m_key;
        private final String m_uri;

        public MenuItem(String key, String uri) {
            m_key = key;
            m_uri = uri;
        }

        public String getKey() {
            return m_key;
        }

        public String getUri() {
            return m_uri;
        }
    }

    public static class AttendantProfileContext extends ProfileContext {
        private final PersonalAttendant m_aa;
        private final String m_domain;

        public AttendantProfileContext(PersonalAttendant aa, String domain, String template) {
            super(null, template);
            m_aa = aa;
            m_domain = domain;
        }

        @Override
        public Map<String, Object> getContext() {
            Map<String, Object> context = super.getContext();
            context.put("user", m_aa.getUser() != null);
            context.put("menu", createMenu());
            context.put("overrideLanguage", m_aa.getOverrideLanguage());
            context.put("language", m_aa.getLanguage());
            context.put("operator", SipUri.fix(m_aa.getOperator(), m_domain));
            return context;
        }

        private List<MenuItem> createMenu() {
            List<MenuItem> menu = new ArrayList<MenuItem>();
            for (Entry<DialPad, AttendantMenuItem> entry : m_aa.getMenu().getMenuItems().entrySet()) {
                String key = entry.getKey().getName();
                String uri = entry.getValue().getParameter();
                uri = SipUri.fix(uri, m_domain);
                menu.add(new MenuItem(key, uri));
            }
            return menu;
        }
    }
}
