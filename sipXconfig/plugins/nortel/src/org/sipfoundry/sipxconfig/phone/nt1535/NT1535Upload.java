/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.nt1535;


import java.io.File;
import java.io.IOException;

import org.apache.commons.io.FileUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.sipfoundry.sipxconfig.upload.Upload;



/**
 * Nortel 1535 device file upload.
 */
public class  NT1535Upload extends Upload {

    private static final Log LOG = LogFactory.getLog(NT1535Upload.class);

    private static final String FROM_SW_VERSION = "firmware/from_sw_version";
    private static final String FROM_HW_VERSION = "firmware/from_hw_version";
    private static final String TO_SW_VERSION = "firmware/to_sw_version";
    private static final String TO_HW_VERSION = "firmware/to_hw_version";
    private static final String FW_UPDATE_REQ_FILE = "FirmwareUpdateRequest.acfg";
    private static final String PATH_DELIM = "/";
    private static final String APPENDIX = "S";

    private String m_profileDir;



    public String getProfileDir() {
        return m_profileDir;
    }

    public void setProfileDir(String profileDir) {
        m_profileDir = profileDir;
    }

    @Override
    public void deploy() {
        super.deploy();
        cpFwUpdateRequestFile();
    }

    @Override
    public void undeploy() {
        rmFwUpdateRequestFile();
        super.undeploy();

    }

    private void cpFwUpdateRequestFile() {

        String fromFileName = m_profileDir + PATH_DELIM
               + getSettingValue(TO_HW_VERSION)
               + PATH_DELIM
               + getSettingValue(TO_SW_VERSION)
               + APPENDIX
               + PATH_DELIM
               + FW_UPDATE_REQ_FILE;

        String toFileName = m_profileDir + PATH_DELIM
               + getSettingValue(FROM_HW_VERSION)
               + PATH_DELIM
               + getSettingValue(FROM_SW_VERSION)
               + APPENDIX
               + PATH_DELIM
               + FW_UPDATE_REQ_FILE;

        File fromFile = new File(fromFileName);
        File toFile = new File(toFileName);

        LOG.debug("NT1535Upload::cpFwUpdateRequestFile from " + fromFileName
                   + " to " + toFileName);

        try {
            FileUtils.copyFile(fromFile, toFile);
        } catch (IOException ext) {

            LOG.error("NT1535Upload failed to copy " + FW_UPDATE_REQ_FILE + " " + ext);

        }
    }

    private void rmFwUpdateRequestFile() {

        String fileName = m_profileDir + PATH_DELIM
            + getSettingValue(FROM_HW_VERSION)
            + PATH_DELIM
            + getSettingValue(FROM_SW_VERSION)
            + APPENDIX
            + PATH_DELIM
            + FW_UPDATE_REQ_FILE;

        LOG.debug("NT1535Upload delete " + fileName);

        File file = new File(fileName);
        FileUtils.deleteQuietly(file);
    }
}
