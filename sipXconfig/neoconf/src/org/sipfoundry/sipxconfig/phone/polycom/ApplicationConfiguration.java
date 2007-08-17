/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.Phone;

/**
 * Velocity model for generating [MAC ADDRESS].cfg, pointer to all other config files. See page 11
 * of Administration guide for more information
 */
public class ApplicationConfiguration extends ProfileContext {
    private String m_serialNumber;

    public ApplicationConfiguration(Phone phone) {
        super(phone, "polycom/mac-address.cfg.vm");
        m_serialNumber = phone.getSerialNumber();
    }

    public String getSipBinaryFilename() {
        return "sip.ld";
    }

    public String getAppFilename() {
        return m_serialNumber + ".cfg";
    }

    public String getSipFilename() {
        return m_serialNumber + "-sipx-sip.cfg";
    }

    public String getPhoneFilename() {
        return m_serialNumber + "-sipx-phone.cfg";
    }

    public String getDirectoryFilename() {
        return m_serialNumber + "-directory.xml";
    }
}
