/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.login;

import java.io.Serializable;
import java.util.Date;

import org.apache.commons.lang.StringUtils;

public class LogFilter implements Serializable {

    private String m_type;
    private Date m_startDate;
    private Date m_endDate;
    private String m_user;
    private String m_remoteIp;

    public LogFilter(String type, Date startDate, Date endDate, String user, String remoteIp) {
        m_type = type;
        m_startDate = startDate;
        m_endDate = endDate;
        m_user = user;
        m_remoteIp = remoteIp;
    }

    public boolean isLogEntryInQuery(LoginEvent loginEvent) {
        return isInDateRange(loginEvent.getDateValue())
                && matchQuery(loginEvent.getType(), m_type)
                && matchQuery(loginEvent.getUserValue(), m_user)
                && matchQuery(loginEvent.getIpValue(), m_remoteIp);
    }

    private boolean matchQuery(String token, String query) {
        if (query == null || StringUtils.trim(token).equals(query)) {
            // filter disabled or query string match
            return true;
        }
        return false;
    }

    private boolean isInDateRange(Date date) {
        if (date == null
                || m_startDate == null
                || m_endDate == null
                || ((m_startDate.getTime() <= date.getTime()) && (m_endDate.getTime() >= date
                        .getTime()))) {
            // return true is filter disabled or date is in range
            return true;
        }

        return false;
    }

}
