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

import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.dialplan.AttendantMenu;

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

    public void setMenu(AttendantMenu menu) {
        m_menu = menu;
    }

    public AttendantMenu getMenu() {
        return m_menu;
    }

    @Override
    public String toString() {
        return "PersonalAttendant [m_menu=" + m_menu + ", m_operator=" + m_operator + ", m_language=" + m_language
                + ", m_overrideLanguage=" + m_overrideLanguage + "]";
    }
}
