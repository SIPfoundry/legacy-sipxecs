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

import org.apache.commons.lang.StringUtils;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Velocity model for generating [MAC ADDRESS].cfg, pointer to all other config files. See page 11
 * of Administration guide for more information
 */
public class ApplicationConfiguration extends ProfileContext<PolycomPhone> {
    public static final String LICENSE_FILE_NAME = "000000000000-license.cfg";

    private final String m_profileDir;

    public ApplicationConfiguration(PolycomPhone phone) {
        super(phone, phone.getModel().getModelDir() + phone.getAppFile());
        m_profileDir = phone.getProfileDir();
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

    public String getCustomConfigs() {
        Setting custom = getDevice().getSettings().getSetting("custom/custom-configs");
        return ApplicationConfiguration.nonBlankEndsInComma(custom.getValue());
    }

    public String getFirmwareFolder() {
        return "polycom/"+getDevice().getDeviceVersion().getVersionId();
    }
    
    /**
     *   transform "abc" goes to "abc," if non-blank
     */
    protected static String nonBlankEndsInComma(String s)  {
        String c =  StringUtils.defaultString(s).trim();
        return StringUtils.isNotBlank(c) && !c.endsWith(",") ? c + ',' : c;
    }

   /**
    * This will list 000000000000-license.cfg and &lt;MAC&gt-license.cfg
    * in the &lt;MAC&gt.cfg if they exist.
    *
    */
    public String getLicenseFileNames() {
        StringBuilder licenseFiles = new StringBuilder();
        licenseFiles.append(ApplicationConfiguration.nonBlankEndsInComma(getUniversalLicenseFilename()));
        licenseFiles.append(ApplicationConfiguration.nonBlankEndsInComma(getMyLicenseFilename()));
        return licenseFiles.toString();

    }
    /**
     * This is to check if universal license file: 000000000000-license.cfg
     * is available under tftproot.
     *
     */
    private String getUniversalLicenseFilename() {

        String licenseFilename = null;
        File licenseFile = new File(m_profileDir + File.separator + LICENSE_FILE_NAME);

        if (licenseFile.exists()) {
            licenseFilename = LICENSE_FILE_NAME;
        }

        return licenseFilename;
    }

    /**
     * This is to check if &lt;MAC&gt-license.cfg is available under tftproot.
     *
     */
    private String getMyLicenseFilename() {

        String licenseFilename = null;
        File licenseFile = new File(m_profileDir + File.separator + getMyLicenseFileName());

        if (licenseFile.exists()) {
            licenseFilename = getMyLicenseFileName();
        }

        return licenseFilename;
    }

    private String getMyLicenseFileName() {
        return getDevice().getProfileFilename() + "-license.cfg";
    }


}
