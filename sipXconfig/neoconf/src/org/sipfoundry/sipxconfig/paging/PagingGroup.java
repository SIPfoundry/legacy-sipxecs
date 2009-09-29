/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.paging;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.User;

public class PagingGroup extends BeanWithId {

    private int m_pageGroupNumber;

    private String m_description;

    private boolean m_enabled = true; // default enabled

    private String m_sound;

    private int m_timeout = 60;       // default to 60 seconds

    private Set<User> m_users = new HashSet<User>();

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public int getPageGroupNumber() {
        return m_pageGroupNumber;
    }

    public void setPageGroupNumber(int number) {
        m_pageGroupNumber = number;
    }

    public String getSound() {
        return m_sound;
    }

    public void setSound(String sound) {
        m_sound = sound;
    }

    public Set<User> getUsers() {
        return m_users;
    }

    public void setUsers(Set<User> users) {
        m_users = users;
    }

    public String formatUserList(String domain) {
        List<String> users = new ArrayList<String>();
        for (User user : m_users) {
            users.add(user.getUserName() + "@" + domain);
        }
        return StringUtils.join(users.toArray(), ',');
    }

    public int getTimeout() {
        return m_timeout;
    }

    public void setTimeout(int timeout) {
        m_timeout = timeout;
    }
}
