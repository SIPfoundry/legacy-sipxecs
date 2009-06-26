/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.logging;

import org.sipfoundry.sipxconfig.admin.commserver.Location;

public interface AuditLogContext {
    void logReplication(String dataName, Location location);
}
