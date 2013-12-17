/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.setting;

import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;

public class LogSettingConfig implements SetupListener {

    private static final String SIP_RELAY_LOG_LEVEL_OLD = "relay-config/SIP_RELAY_LOG_LEVEL";
    private static final String SIP_RELAY_LOG_LEVEL_NEW = "relay-config/log.level";

    private SettingDao m_settingDao;

    @Override
    public boolean setup(SetupManager manager) {
        if (manager.isFalse(SIP_RELAY_LOG_LEVEL_NEW)) {
            m_settingDao.updateSettingName(SIP_RELAY_LOG_LEVEL_OLD, SIP_RELAY_LOG_LEVEL_NEW);
        }
        manager.setTrue(SIP_RELAY_LOG_LEVEL_NEW);
        return true;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

}
