/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.api.impl;

import org.sipfoundry.sipxconfig.api.FtpApi;
import org.sipfoundry.sipxconfig.ftp.FtpManager;
import org.sipfoundry.sipxconfig.ftp.FtpSettings;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;

public class FtpApiImpl extends BaseServiceApiImpl implements FtpApi {
    private FtpManager m_ftpManager;


    public void setFtpManager(FtpManager manager) {
        m_ftpManager = manager;
    }

    @Override
    protected PersistableSettings getSettings() {
        return m_ftpManager.getSettings();
    }

    @Override
    protected void saveSettings(PersistableSettings settings) {
        m_ftpManager.saveSettings((FtpSettings) settings);
    }

}
