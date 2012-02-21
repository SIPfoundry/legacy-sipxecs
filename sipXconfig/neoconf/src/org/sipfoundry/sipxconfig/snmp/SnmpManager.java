/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.snmp;

import java.util.List;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;

public interface SnmpManager {
    public static final GlobalFeature FEATURE = new GlobalFeature("snmp");

    public List<ProcessDefinition> getProcessDefinitions(Location location);

    public List<ServiceStatus> getServicesStatuses(Location location);
}
