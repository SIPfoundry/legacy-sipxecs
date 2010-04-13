/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.linksys;

import org.sipfoundry.sipxconfig.phone.PhoneModel;

import static org.apache.commons.lang.StringUtils.isBlank;

/**
 * Static differences in linksys models
 */
public class LinksysModel extends PhoneModel {

    private String m_psn;

    public LinksysModel() {
    }

    public LinksysModel(String beanId) {
        super(beanId);
    }

    /**
     * Linksys phones by default look for a minimum initial configuration file, which in turns
     * provide the name of the specific configuration file.
     *
     * The name of the file seems to depend on the device model. Most documents on the web suggest
     * it's 'spa$PSN.cfg' where $PSN refers to numeric portion of the model name.
     */
    public String getDefaultConfigName() {
        if (isBlank(m_psn)) {
            return null;
        }
        return String.format("spa%s.cfg", m_psn);
    }

    public void setPsn(String psn) {
        m_psn = psn;
    }
}
