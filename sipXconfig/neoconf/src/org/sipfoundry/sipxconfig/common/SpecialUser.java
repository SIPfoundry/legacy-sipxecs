/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.common;

import org.apache.commons.lang.RandomStringUtils;

public enum SpecialUser {
    PARK_SERVER("~~id~park"), MEDIA_SERVER("~~id~media");

    private String m_id;
    private String m_sipPassword = RandomStringUtils.randomAlphanumeric(10);

    SpecialUser(String id) {
        m_id = id;
    }

    String getId() {
        return m_id;
    }

    public String getSipPassword() {
        return m_sipPassword;
    }
}
