/**
 *
 *
 * Copyright (c) 2014 Karel, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.systemaudit;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.dialplan.AutoAttendantManager;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbit;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SystemAuditLocalizationProviderImpl implements SystemAuditLocalizationProvider {

    protected static final Logger LOG = Logger.getLogger(SystemAuditLocalizationProviderImpl.class);

    private PermissionManager m_permissionManager;
    private PhoneContext m_phoneContext;
    private ConferenceBridgeContext m_conferenceBridgeContext;
    private AutoAttendantManager m_autoAttendantManager;
    private ParkOrbitContext m_parkOrbitContext;

    @Override
    public Setting getLocalizedSetting(ConfigChange configChange, String propertyName, String value) {
        Setting setting = null;
        if (configChange.getConfigChangeType() == ConfigChangeType.USER) {
            User user = new User();
            user.setPermissionManager(m_permissionManager);
            setting = user.getSettings().getSetting(propertyName);
        } else if (configChange.getConfigChangeType() == ConfigChangeType.PHONE) {
            Phone phone = m_phoneContext.getPhoneBySerialNumber(configChange
                    .getDetails());
            setting = phone.getSettings().getSetting(propertyName);
        } else if (configChange.getConfigChangeType() == ConfigChangeType.CONFERENCE) {
            Conference conference = m_conferenceBridgeContext
                    .findConferenceByName(configChange.getDetails());
            setting = conference.getSettings().getSetting(propertyName);
        } else if (configChange.getConfigChangeType() == ConfigChangeType.AUTO_ATTENDANT) {
            AutoAttendant aa = m_autoAttendantManager
                    .getAutoAttendantBySystemName(configChange.getDetails());
            setting = aa.getSettings().getSetting(propertyName);
        } else if (configChange.getConfigChangeType() == ConfigChangeType.CALL_PARK) {
            ParkOrbit parkOrbit = m_parkOrbitContext
                    .loadParkOrbitByName(configChange.getDetails());
            setting = parkOrbit.getSettings().getSetting(propertyName);
        }
        return setting;
    }

    @Override
    public BeanWithSettings getLocalizedBeanWithSettings(ConfigChange configChange, String message) {
        if (configChange.getConfigChangeType() == ConfigChangeType.USER) {
            User user = new User();
            user.setPermissionManager(m_permissionManager);
            return user;
        } else if (configChange.getConfigChangeType() == ConfigChangeType.PHONE) {
            Phone phone = m_phoneContext.getPhoneBySerialNumber(configChange
                    .getDetails());
            return phone;
        } else if (configChange.getConfigChangeType() == ConfigChangeType.CONFERENCE) {
            Conference conference = m_conferenceBridgeContext
                    .findConferenceByName(configChange.getDetails());
            return conference;
        } else if (configChange.getConfigChangeType() == ConfigChangeType.AUTO_ATTENDANT) {
            AutoAttendant aa = m_autoAttendantManager
                    .getAutoAttendantByName(configChange.getDetails());
            return aa;
        } else if (configChange.getConfigChangeType() == ConfigChangeType.CALL_PARK) {
            ParkOrbit parkOrbit = m_parkOrbitContext
                    .loadParkOrbitByName(configChange.getDetails());
            return parkOrbit;
        }
        return null;
    }

    @Override
    public String getLocalizedPropertyName(ConfigChange configChange,
            String propertyName, String value) {
        return null;
    }
}
