/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.admin.commserver.Location;

public interface ConfigVersionManager {
    /**
     * Updates configuration version for a service running an a specific location.
     *
     * That should be called after entire service configuration is successfully replicated. Can be
     * safely called multiple times - the version on the server will only be updated if it's
     * actually different.
     */
    void setConfigVersion(SipxService service, Location location);
}
