/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.springframework.beans.factory.annotation.Required;

public class SipxFreeswitchService extends SipxService {
    public static final String BEAN_ID = "sipxFreeswitchService";

    private String m_docDir;
    
    @Required
    public void setDocDir(String docDir) {
        m_docDir = docDir;
    }
    
    public String getDocDir() {
        return m_docDir;
    }
}
