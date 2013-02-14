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
package org.sipfoundry.sipxconfig.fail2ban;

import org.sipfoundry.sipxconfig.alarm.AlarmDefinition;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;

public interface Fail2banManager {
    public static final GlobalFeature FEATURE = new GlobalFeature("fail2ban");
    public static final AlarmDefinition SECURITY_IP_BANNED = new AlarmDefinition("SECURITY_IP_BANNED");
    public static final AlarmDefinition SECURITY_IP_UNBANNED = new AlarmDefinition("SECURITY_IP_UNBANNED");

    Fail2banSettings getSettings();

    void saveSettings(Fail2banSettings settings);

}
