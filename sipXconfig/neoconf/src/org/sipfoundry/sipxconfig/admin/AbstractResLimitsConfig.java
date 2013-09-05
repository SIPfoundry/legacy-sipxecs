/**
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
package org.sipfoundry.sipxconfig.admin;

import java.io.IOException;
import java.io.Writer;

import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
/**
 * Generic res limits configurer
 * Extensions of this class should provide the process prefix: sipxproxy-; sipxpark- etc...
 */
public abstract class AbstractResLimitsConfig {

    public abstract String getPrefix();

    public void writeResourceLimits(Writer w, PersistableSettings settings) throws IOException {
        KeyValueConfiguration resLimits = KeyValueConfiguration.equalsSeparated(w);
        Setting fdSoft = settings.getSettings().getSetting("resource-limits/fd-soft");
        Setting fdHard = settings.getSettings().getSetting("resource-limits/fd-hard");
        Setting coreEnabled = settings.getSettings().getSetting("resource-limits/core-enabled");
        resLimits.write(getPrefix(), fdSoft.getName(), fdSoft.getValue());
        resLimits.write(getPrefix(), fdHard.getName(), fdHard.getValue());
        resLimits.write(getPrefix(), coreEnabled.getName(), coreEnabled.getValue());
    }
}
