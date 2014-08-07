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

import java.util.List;

import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public interface SystemAuditLocalizationProvider {

    public Setting getLocalizedSetting(ConfigChange configChange, String propertyName, String value);

    public BeanWithSettings getLocalizedBeanWithSettings(ConfigChange configChange, String message);

    public String getLocalizedPropertyName(ConfigChange configChange, String propertyName, String value);

    /**
     * Returns an array ith the package names it will scan to detect
     * ConfigChangeTypes
     */
    public List<String> getPackageNamesForSystemAudit();

    /**
     * Builds a String array with all the ConfigChangeTypes it will find in the
     * ClassLoader it uses @getPackageNamesForSystemAudit for scanning classes
     */
    public String[] getConfigChangeTypeArray();
}
