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

import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public interface SystemAuditLocalizationProvider {

    public Setting getLocalizedSetting(ConfigChange configChange, String propertyName, String value);

    public BeanWithSettings getLocalizedBeanWithSettings(ConfigChange configChange, String message);

    public String getLocalizedPropertyName(ConfigChange configChange, String propertyName, String value);
}
