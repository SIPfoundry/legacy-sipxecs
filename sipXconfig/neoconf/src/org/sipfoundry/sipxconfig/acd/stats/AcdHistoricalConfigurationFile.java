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

import java.util.List;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.admin.TemplateConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class AcdHistoricalConfigurationFile extends TemplateConfigurationFile {

    private static final String EMPTY_STRING = "";
    private static final String HOSTNAME_SEPERATOR = ";";

    private String m_dbUser;
    private int m_agentPort;
    private AcdContext m_acdContext;

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        //Default locationFqdn is "localhost"
        context.put("dbUser", m_dbUser);
        List<AcdServer> acdServers = m_acdContext.getServers();

        String locationFqdn = EMPTY_STRING;
        String configServerAgentUrl = EMPTY_STRING;

        if (0 == acdServers.size()) {
            locationFqdn += "localhost";
            configServerAgentUrl += "http://localhost:" + String.valueOf(m_agentPort);
        } else {
            for (AcdServer acdServer : acdServers) {
                locationFqdn += acdServer.getLocation().getFqdn() + HOSTNAME_SEPERATOR;
                configServerAgentUrl += "http://" + acdServer.getLocation().getFqdn()
                    + ":" + String.valueOf(m_agentPort) + HOSTNAME_SEPERATOR;
            }
        }

        context.put("locationFqdn", locationFqdn);
        context.put("configServerAgentUrl", configServerAgentUrl);
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

    public void setAgentPort(int agentPort) {
        m_agentPort = agentPort;
    }

    public void setAcdContext(AcdContext acdContext) {
        m_acdContext = acdContext;
    }
}
