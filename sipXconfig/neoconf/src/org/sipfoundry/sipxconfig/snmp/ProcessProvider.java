/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.snmp;

import java.util.Collection;

import org.sipfoundry.sipxconfig.commserver.Location;

public interface ProcessProvider {

    Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location);

}
