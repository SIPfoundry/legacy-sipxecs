package org.sipfoundry.sipxconfig.phone.polycom;

import org.sipfoundry.sipxconfig.upload.Upload;

public class PolycomUpload extends Upload {
    private static final String POLYCOM_DIR = "/polycom/";
    private static final String VERSION = "firmware/version";
    private String m_profileDir;

    public String getProfileDir() {
        return m_profileDir;
    }

    public void setProfileDir(String profileDir) {
        m_profileDir = profileDir;
    }

    @Override
    public void deploy() {
        super.setDestinationDirectory(getDestinationDirectory() + POLYCOM_DIR + getSettingValue(VERSION));
        super.deploy();
    }

    @Override
    public void undeploy() {
        super.setDestinationDirectory(getDestinationDirectory() + POLYCOM_DIR + getSettingValue(VERSION));
        super.undeploy();
    }

}
