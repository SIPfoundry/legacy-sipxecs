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

import java.io.File;

import org.sipfoundry.sipxconfig.device.ProfileContext;

/**
 * Velocity model for generating [MAC ADDRESS].cfg, pointer to all other config files. See page 11
 * of Administration guide for more information
 */
public class ApplicationConfiguration extends ProfileContext<PolycomPhone> {
    public static final String LICENSE_FILE_NAME = "000000000000-license.cfg";

    private final String m_profileDir;

    public ApplicationConfiguration(PolycomPhone phone) {
        super(phone, "polycom/mac-address.cfg.vm");
        m_profileDir = phone.getProfileDir();
    }

    public String getSipBinaryFilename() {
        return "sip.ld";
    }

    public String getAppFilename() {
        return getDevice().getAppFilename();
    }

    public String getSipFilename() {
        return getDevice().getSipFilename();
    }

    public String getPhoneFilename() {
        return getDevice().getPhoneFilename();
    }

    public String getDirectoryFilename() {
        return getDevice().getDirectoryFilename();
    }

    public String getDeviceFilename() {
        return getDevice().getDeviceFilename();
    }

    /**
     * This is to check if 000000000000-license.cfg is available.
     */
    public String getLicenseFilename() {

        String licenseFilename = null;
        File licenseFile = new File(m_profileDir + "/" + LICENSE_FILE_NAME);

        if (licenseFile.exists()) {
            licenseFilename = LICENSE_FILE_NAME;
        }

        return licenseFilename;
    }
}
