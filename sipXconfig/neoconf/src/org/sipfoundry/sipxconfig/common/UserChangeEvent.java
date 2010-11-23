/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.common;

import org.springframework.context.ApplicationEvent;

public class UserChangeEvent extends ApplicationEvent {

    private Integer m_userId;
    //The username of the user that is about to get changed
    private String m_oldUserName;
    private String m_userName;
    private String m_firstName;
    private String m_lastName;

    public UserChangeEvent(Object source, Integer userId, String oldUserName, String userName, String firstName,
            String lastName) {
        super(source);
        m_userId = userId;
        m_oldUserName = oldUserName;
        m_userName = userName;
        m_firstName = firstName;
        m_lastName = lastName;
    }

    public Integer getUserId() {
        return m_userId;
    }

    public void setUserId(Integer userId) {
        m_userId = userId;
    }

    public String getOldUserName() {
        return m_oldUserName;
    }

    public void setOldUserName(String oldUserName) {
        m_oldUserName = oldUserName;
    }

    public String getUserName() {
        return m_userName;
    }

    public void setUserName(String userName) {
        m_userName = userName;
    }

    public String getFirstName() {
        return m_firstName;
    }

    public void setFirstName(String firstName) {
        m_firstName = firstName;
    }

    public String getLastName() {
        return m_lastName;
    }

    public void setLastName(String lastName) {
        m_lastName = lastName;
    }

}
