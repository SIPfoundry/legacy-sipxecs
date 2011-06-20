/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.cisco;

import org.sipfoundry.sipxconfig.phone.PhoneModel;

/**
 * Static differences in cisco models
 */
public final class CiscoModel extends PhoneModel {
    private String m_cfgPrefix;

    private String m_upgCode;

    public CiscoModel() {
    }

    public CiscoModel(String beanId) {
        super(beanId);
    }

    public String getCfgPrefix() {
        return m_cfgPrefix;
    }

    public String getUpgCode() {
        return m_upgCode;
    }

    public boolean isAta() {
        return "ata".equals(getCfgPrefix());
    }

    public void setCfgPrefix(String cfgPrefix) {
        m_cfgPrefix = cfgPrefix;
    }

    public void setUpgCode(String upgCode) {
        m_upgCode = upgCode;
    }
}
