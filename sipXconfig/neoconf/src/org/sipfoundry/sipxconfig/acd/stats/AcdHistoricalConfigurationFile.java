/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd.stats;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.TemplateConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;

public class AcdHistoricalConfigurationFile extends TemplateConfigurationFile {
    private String m_dbUser;
    private LocationsManager m_locationsManager;

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        //Default locationFqdn is "localhost"
        context.put("dbUser", m_dbUser);
        Location callCenterLocation = m_locationsManager.getCallCenterLocation();
        String locationFqdn = (callCenterLocation == null) ? "localhost" : callCenterLocation.getFqdn();
        context.put("locationFqdn", locationFqdn);
        context.put("dbUri", "dbi:Pg:#{DB_NAME}:localhost");
        return context;
    }

    /**
     * This is replicable only on primary location where the report component is installed
     */
    public boolean isReplicable(Location location) {
        return location.isPrimary();
    }

    public void setDbUser(String dbUser) {
        m_dbUser = dbUser;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

}
