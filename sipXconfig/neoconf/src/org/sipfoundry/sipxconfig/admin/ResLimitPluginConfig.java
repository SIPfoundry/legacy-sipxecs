/**
 *
 *
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
package org.sipfoundry.sipxconfig.admin;

import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
/**
 * This interface could be implemented by concrete ConfigProvider(s) that need to
 * replicate resource limits data - see ResLimitsConfiguration and resource-limits values per process
 * So, if you have a service plugin it can bring its own resource limits
 */
public interface ResLimitPluginConfig {
    LocationFeature getLocationFeature();
    AbstractResLimitsConfig getLimitsConfig();
    PersistableSettings getSettings();
    void saveSettings(PersistableSettings settings);
}
