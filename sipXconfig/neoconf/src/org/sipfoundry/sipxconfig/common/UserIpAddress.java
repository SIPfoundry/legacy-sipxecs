/**
 *
 * Copyright (c) 2013 Karel Electronics Corp. All rights reserved.
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
 *
 */

package org.sipfoundry.sipxconfig.common;

public class UserIpAddress extends BeanWithId {

    private String m_userName;
    private String m_ipAddress;

    public UserIpAddress(String userName, String ipAddress) {
        super();
        this.m_userName = userName;
        this.m_ipAddress = ipAddress;
    }

    public UserIpAddress() {
        super();
    }

    public String getUserName() {
        return m_userName;
    }

    public void setUserName(String userName) {
        this.m_userName = userName;
    }

    public String getIpAddress() {
        return m_ipAddress;
    }

    public void setIpAddress(String ipAddress) {
        this.m_ipAddress = ipAddress;
    }

}
