/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.paging;

import org.sipfoundry.sipxconfig.common.BeanWithId;

public class PagingServer extends BeanWithId {
    private String m_prefix;

    private String m_logLevel;

    private String m_sipTraceLevel;

    public String getPrefix() {
        return m_prefix;
    }

    public void setPrefix(String prefix) {
        m_prefix = prefix;
    }

    public String getLogLevel() {
        return m_logLevel;
    }

    public void setLogLevel(String logLevel) {
        m_logLevel = logLevel;
    }

    public String getSipTraceLevel() {
        return m_sipTraceLevel;
    }

    public void setSipTraceLevel(String sipTraceLevel) {
        m_sipTraceLevel = sipTraceLevel;
    }
}
