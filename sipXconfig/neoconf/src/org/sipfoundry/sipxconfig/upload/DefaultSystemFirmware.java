/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.upload;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;

public class DefaultSystemFirmware {

    private static final String TRUE = "true";

    private final String m_uploadSpec;
    private final String m_version;
    private final String m_defaultActivation;
    private final String m_overridePreviousFirmwares;
    private final List<String> m_fileSettings;
    private final List<String> m_uploadFiles;

    public DefaultSystemFirmware(String uploadSpec, List<String> fileSettings, List<String> uploadFiles,
            String defaultActivation, String overridePreviousFirmwares, String version) {
        m_uploadSpec = uploadSpec;
        m_fileSettings = fileSettings;
        m_uploadFiles = uploadFiles;
        m_version = version;
        m_defaultActivation = defaultActivation;
        m_overridePreviousFirmwares = overridePreviousFirmwares;
    }

    public boolean isActivateByDefault() {
        return null == m_defaultActivation ? false : 0 == m_defaultActivation.compareToIgnoreCase(TRUE);
    }

    public boolean isOverridePreviousFirmwares() {
        return null == m_overridePreviousFirmwares ? false : 0 == m_overridePreviousFirmwares
                .compareToIgnoreCase(TRUE);
    }

    public String getVersion() {
        return m_version;
    }

    public String getUploadSpec() {
        return m_uploadSpec;
    }

    public List<String> getFileSettings() {
        return m_fileSettings;
    }

    public List<String> getUploadFiles() {
        return m_uploadFiles;
    }

    public String[] getUploadArgs() {
        List<String> args = new ArrayList<String>();
        args.add(StringUtils.EMPTY);
        args.add(m_uploadSpec);

        if (null != m_fileSettings && null != m_uploadFiles) {
            final int len = Math.min(m_fileSettings.size(), m_uploadFiles.size());

            for (int i = 0; i < len; i++) {
                final String firmwareFileName = m_uploadFiles.get(i);
                final File firmwareFile = new File(firmwareFileName);
                if (firmwareFile.exists()) {
                    args.add(m_fileSettings.get(i));
                    args.add(firmwareFileName);
                }
            }
        }

        return args.toArray(new String[args.size()]);
    }
}
