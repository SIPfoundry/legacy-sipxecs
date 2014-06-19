/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.api.model;

import java.util.ArrayList;
import java.util.List;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElementWrapper;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlType;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.paging.PagingGroup;

@XmlRootElement(name = "Group")
@XmlType(propOrder = {
        "id", "enabled", "pageGroupNumber", "timeout", "sound", "description", "users"
        })
public class PageGroupBean {
    private int m_id;
    private boolean m_enabled;
    private int m_pageGroupNumber;
    private int m_timeout;
    private String m_sound;
    private String m_description;
    private List<UserBean> m_users;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

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

    public int getTimeout() {
        return m_timeout;
    }

    public void setTimeout(int timeout) {
        m_timeout = timeout;
    }

    public void setUsers(List<UserBean> users) {
        m_users = users;
    }

    @XmlElementWrapper(name = "Users")
    @XmlElement(name = "User")
    public List<UserBean> getUsers() {
        return m_users;
    }

    public static PageGroupBean convertGroup(PagingGroup group) {
        if (group == null) {
            return null;
        }
        PageGroupBean bean = new PageGroupBean();
        bean.setId(group.getId());
        bean.setEnabled(group.isEnabled());
        bean.setDescription(group.getDescription());
        bean.setPageGroupNumber(group.getPageGroupNumber());
        bean.setSound(group.getSound());
        bean.setTimeout(group.getTimeout());
        List<UserBean> userBeans = new ArrayList<UserBean>();
        for (User user : group.getUsers()) {
            UserBean userBean = new UserBean();
            userBean.setId(user.getId());
            userBean.setUserName(user.getUserName());
            userBean.setLastName(user.getLastName());
            userBean.setFirstName(user.getFirstName());
            List<String> aliases = new ArrayList<String>();
            for (String alias : user.getAliases()) {
                aliases.add(alias);
            }
            userBean.setAliases(aliases);
            userBeans.add(userBean);
        }
        if (!userBeans.isEmpty()) {
            bean.setUsers(userBeans);
        }
        return bean;
    }

    public static void populateGroup(PageGroupBean bean, PagingGroup group) {
        group.setDescription(bean.getDescription());
        group.setEnabled(bean.isEnabled());
        group.setPageGroupNumber(bean.getPageGroupNumber());
        group.setSound(bean.getSound());
        group.setTimeout(bean.getTimeout());
    }

    @XmlRootElement(name = "User")
    @XmlType(propOrder = {
            "id", "userName", "lastName", "firstName", "aliases"
            })
    public static class UserBean {
        private Integer m_id;
        private String m_userName;
        private String m_lastName;
        private String m_firstName;
        private List<String> m_aliases;

        public Integer getId() {
            return m_id;
        }

        public void setId(Integer id) {
            m_id = id;
        }

        public String getUserName() {
            return m_userName;
        }

        public void setUserName(String name) {
            m_userName = name;
        }

        public String getLastName() {
            return m_lastName;
        }

        public void setLastName(String name) {
            m_lastName = name;
        }

        public String getFirstName() {
            return m_firstName;
        }

        public void setFirstName(String name) {
            m_firstName = name;
        }

        public void setAliases(List<String> aliases) {
            m_aliases = aliases;
        }

        @XmlElementWrapper(name = "Aliases")
        @XmlElement(name = "Alias")
        public List<String> getAliases() {
            return m_aliases;
        }
    }
}
