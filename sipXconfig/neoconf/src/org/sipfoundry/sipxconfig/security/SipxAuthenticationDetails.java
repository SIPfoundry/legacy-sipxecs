/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.security;

import java.io.Serializable;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpSession;

import org.acegisecurity.concurrent.SessionIdentifierAware;

// Adapted from Acegi's WebAuthenticationDetails
public class SipxAuthenticationDetails implements SessionIdentifierAware, Serializable {
    private String m_remoteAddress;
    private String m_sessionId;

    public SipxAuthenticationDetails(HttpServletRequest request) {
        String realIp = request.getHeader("x-forwarded-for");
        if (realIp != null) {
            m_remoteAddress = realIp;
        }

        HttpSession session = request.getSession(false);
        m_sessionId = (session != null) ? session.getId() : null;
    }

    @Override
    public String getSessionId() {
        return m_sessionId;
    }

    public boolean equals(Object obj) {
        if (obj instanceof SipxAuthenticationDetails) {
            SipxAuthenticationDetails rhs = (SipxAuthenticationDetails) obj;
            if ((m_remoteAddress == null) && (rhs.getRemoteAddress() != null)) {
                return false;
            }
            if ((m_remoteAddress != null) && (rhs.getRemoteAddress() == null)) {
                return false;
            }
            if (m_remoteAddress != null) {
                if (!m_remoteAddress.equals(rhs.getRemoteAddress())) {
                    return false;
                }
            }
            if ((m_sessionId == null) && (rhs.getSessionId() != null)) {
                return false;
            }
            if ((m_sessionId != null) && (rhs.getSessionId() == null)) {
                return false;
            }
            if (m_sessionId != null) {
                if (!m_sessionId.equals(rhs.getSessionId())) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    public String getRemoteAddress() {
        return m_remoteAddress;
    }

    public int hashCode() {
        int code = 7654;
        if (m_remoteAddress != null) {
            code = code * (m_remoteAddress.hashCode() % 7);
        }
        if (m_sessionId != null) {
            code = code * (m_sessionId.hashCode() % 7);
        }
        return code;
    }

    public String toString() {
        StringBuffer sb = new StringBuffer();
        sb.append(super.toString() + ": ");
        sb.append("RemoteIpAddress: " + getRemoteAddress() + "; ");
        sb.append("SessionId: " + getSessionId());

        return sb.toString();
    }

}
