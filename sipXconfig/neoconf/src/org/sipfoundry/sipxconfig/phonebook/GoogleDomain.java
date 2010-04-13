/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.phonebook;

import org.sipfoundry.sipxconfig.common.BeanWithId;

public class GoogleDomain extends BeanWithId {

    private String m_domainName;

    public GoogleDomain() {
    }

    public String getDomainName() {
        return m_domainName;
    }

    public void setDomainName(String name) {
        m_domainName = name;
    }
}
