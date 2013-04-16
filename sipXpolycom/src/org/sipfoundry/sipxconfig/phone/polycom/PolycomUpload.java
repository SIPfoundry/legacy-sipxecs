/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
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
