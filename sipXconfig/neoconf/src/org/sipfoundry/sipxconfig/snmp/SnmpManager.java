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
package org.sipfoundry.sipxconfig.snmp;

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;

public interface SnmpManager {
    public static final GlobalFeature FEATURE = new GlobalFeature("snmp");

    public SnmpSettings getSettings();

    public void saveSettings(SnmpSettings settings);

    public FeatureManager getFeatureManager();

    public List<ProcessDefinition> getProcessDefinitions(Location location);

    public List<ServiceStatus> getServicesStatuses(Location location);

    public List<ProcessDefinition> getProcessDefinitions(Location location, Collection<String> processId);

    public void restartProcesses(Location location, Collection<ProcessDefinition> processes);
}
