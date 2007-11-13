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

import java.io.File;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.User;

public class PagingGroup extends BeanWithId {

    private Long m_pageGroupNumber;

    private String m_description;

    private boolean m_enabled;

    private String m_sound;

    private Set<User> m_users = new HashSet<User>();

    private String m_prefix;

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

    public Long getPageGroupNumber() {
        return m_pageGroupNumber;
    }

    public void setPageGroupNumber(Long number) {
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

    public String getPrefix() {
        return m_prefix;
    }

    public void setPrefix(String prefix) {
        m_prefix = prefix;
    }

    public String formatUrls(String domain) {
        List<String> users = new ArrayList<String>();
        Iterator<User> it = m_users.iterator();
        while (it.hasNext()) {
            User user = (User) it.next();
            users.add(user.getUserName() + "@" + domain);
        }
        return StringUtils.join(users.toArray(), ',');
    }

    public String formatBeep(String audioDirectory) {
        File musicFile = new File(audioDirectory, m_sound);
        return "file://" + musicFile.getPath();
    }

    public String formatDescription() {
        return m_description;
    }

    public String formatPageGroupNumber() {
        return String.valueOf(m_pageGroupNumber);
    }
}
