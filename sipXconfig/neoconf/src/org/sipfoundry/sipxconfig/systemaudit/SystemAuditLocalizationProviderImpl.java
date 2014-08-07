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

import java.util.ArrayList;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import org.apache.log4j.Logger;
import org.reflections.Reflections;
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
import org.springframework.beans.factory.annotation.Required;

public class SystemAuditLocalizationProviderImpl implements SystemAuditLocalizationProvider {

    protected static final Logger LOG = Logger.getLogger(SystemAuditLocalizationProviderImpl.class);

    private PermissionManager m_permissionManager;
    private PhoneContext m_phoneContext;
    private ConferenceBridgeContext m_conferenceBridgeContext;
    private AutoAttendantManager m_autoAttendantManager;
    private ParkOrbitContext m_parkOrbitContext;

    @Override
    public Setting getLocalizedSetting(ConfigChange configChange,
            String propertyName, String value) {
        Setting setting = null;
        if (configChange.getConfigChangeType().equals(
                ConfigChangeType.USER.getName())) {
            User user = new User();
            user.setPermissionManager(m_permissionManager);
            setting = user.getSettings().getSetting(propertyName);
        } else if (configChange.getConfigChangeType().equals(
                ConfigChangeType.PHONE.getName())) {
            Phone phone = m_phoneContext.getPhoneBySerialNumber(configChange
                    .getDetails());
            setting = phone.getSettings().getSetting(propertyName);
        } else if (configChange.getConfigChangeType().equals(
                new Conference().getConfigChangeType())) {
            Conference conference = m_conferenceBridgeContext
                    .findConferenceByName(configChange.getDetails());
            setting = conference.getSettings().getSetting(propertyName);
        } else if (configChange.getConfigChangeType().equals(
                new AutoAttendant().getConfigChangeType())) {
            AutoAttendant aa = m_autoAttendantManager
                    .getAutoAttendantBySystemName(configChange.getDetails());
            setting = aa.getSettings().getSetting(propertyName);
        } else if (configChange.getConfigChangeType().equals(
                new ParkOrbit().getConfigChangeType())) {
            ParkOrbit parkOrbit = m_parkOrbitContext
                    .loadParkOrbitByName(configChange.getDetails());
            setting = parkOrbit.getSettings().getSetting(propertyName);
        }
        return setting;
    }

    @Override
    public BeanWithSettings getLocalizedBeanWithSettings(
            ConfigChange configChange, String message) {
        if (configChange.getConfigChangeType().equals(
                ConfigChangeType.USER.getName())) {
            User user = new User();
            user.setPermissionManager(m_permissionManager);
            return user;
        } else if (configChange.getConfigChangeType().equals(
                ConfigChangeType.PHONE.getName())) {
            Phone phone = m_phoneContext.getPhoneBySerialNumber(configChange
                    .getDetails());
            return phone;
        } else if (configChange.getConfigChangeType().equals(
                new Conference().getConfigChangeType())) {
            Conference conference = m_conferenceBridgeContext
                    .findConferenceByName(configChange.getDetails());
            return conference;
        } else if (configChange.getConfigChangeType().equals(
                new AutoAttendant().getConfigChangeType())) {
            AutoAttendant aa = m_autoAttendantManager
                    .getAutoAttendantByName(configChange.getDetails());
            return aa;
        } else if (configChange.getConfigChangeType().equals(
                new ParkOrbit().getConfigChangeType())) {
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

    @Required
    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }

    @Required
    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    @Required
    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }

    @Required
    public void setAutoAttendantManager(AutoAttendantManager autoAttendantManager) {
        m_autoAttendantManager = autoAttendantManager;
    }

    @Required
    public void setParkOrbitContext(ParkOrbitContext parkOrbitContext) {
        m_parkOrbitContext = parkOrbitContext;
    }

    @Override
    public List<String> getPackageNamesForSystemAudit() {
        List<String> packageNames = new ArrayList<String>();
        packageNames.add("org.sipfoundry.sipxconfig");
        return packageNames;
    }

    @Override
    public String[] getConfigChangeTypeArray() {
        Set<String> configChangeTypes = new LinkedHashSet<String>(
                ConfigChangeType.getValues());
        for (String packageName : getPackageNamesForSystemAudit()) {
            configChangeTypes
                    .addAll(getConfigChangeTypesFromPackage(packageName));
        }
        return configChangeTypes.toArray(new String[] {});
    }

    private Set<String> getConfigChangeTypesFromPackage(String packageName) {
        Reflections reflections = new Reflections(packageName);
        Set<Class<? extends SystemAuditable>> systemAuditableClasses = reflections
                .getSubTypesOf(SystemAuditable.class);
        Set<String> configChangeTypes = new HashSet<String>();
        for (Class<? extends SystemAuditable> sa : systemAuditableClasses) {
            SystemAuditable instantiatedSystemAuditable = null;
            try {
                instantiatedSystemAuditable = sa.newInstance();
            } catch (Exception e) {
                // Do nothing, we expect some classes not to be initialized
                LOG.debug(e);
            }
            if (instantiatedSystemAuditable != null) {
                configChangeTypes.add(instantiatedSystemAuditable
                        .getConfigChangeType());
            }
        }
        return configChangeTypes;
    }

}
