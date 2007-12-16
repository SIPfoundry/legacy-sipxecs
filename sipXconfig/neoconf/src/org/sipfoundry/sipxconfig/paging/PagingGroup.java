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
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.User;

public class PagingGroup extends BeanWithId {

    static final String URLS_KEY_FORMAT = "page.group.%d.urls";
    static final String USER_KEY_FORMAT = "page.group.%d.user";
    static final String BEEP_KEY_FORMAT = "page.group.%d.beep";
    static final String DESCRIPTION_KEY_FORMAT = "page.group.%d.description";

    private Long m_pageGroupNumber;

    private String m_description;

    private boolean m_enabled;

    private String m_sound;

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

    String formatUrls(String domain) {
        List<String> users = new ArrayList<String>();
        for (User user : m_users) {
            users.add(user.getUserName() + "@" + domain);
        }
        return StringUtils.join(users.toArray(), ',');
    }

    String formatBeep(String audioDirectory) {
        File musicFile = new File(audioDirectory, m_sound);
        return "file://" + musicFile.getPath();
    }

    public void addProperties(Map<String, String> config, int index, String audioDir,
            String domain) {
        config.put(String.format(DESCRIPTION_KEY_FORMAT, index), StringUtils
                .defaultString(m_description));
        config.put(String.format(BEEP_KEY_FORMAT, index), formatBeep(audioDir));
        config.put(String.format(USER_KEY_FORMAT, index), String.valueOf(m_pageGroupNumber));
        config.put(String.format(URLS_KEY_FORMAT, index), formatUrls(domain));
    }
}
