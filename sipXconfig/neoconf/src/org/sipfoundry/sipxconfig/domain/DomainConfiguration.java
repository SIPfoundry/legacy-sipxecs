/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import java.util.LinkedHashSet;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.TemplateConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;

public class DomainConfiguration extends TemplateConfigurationFile {

    public static final char SEPARATOR_CHAR = ' ';
    private Domain m_domain;
    private String m_language;
    private String m_configServerHost;
    private LocationsManager m_locationsManager;

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void generate(Domain domain, String configServerHost, String language) {
        m_domain = domain;
        m_language = language;
        m_configServerHost = configServerHost;
    }

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        context.put("domain", m_domain);
        context.put("domainAliases", getDomainAliases(m_domain));
        context.put("language", m_language);
        context.put("configServerHost", m_configServerHost);
        return context;
    }

    /**
     * Generate list of aliases required by SIP_DOMAIN_ALIASES all domain aliases configured by
     * user and IP addresses and FQDN for all servers
     */
    private String getDomainAliases(Domain domain) {
        Set<String> aliases = new LinkedHashSet<String>();
        Set<String> configuredAliases = domain.getAliases();
        if (configuredAliases != null) {
            aliases.addAll(configuredAliases);
        }

        Location[] locations = m_locationsManager.getLocations();
        for (Location location : locations) {
            aliases.add(location.getAddress());
            aliases.add(location.getFqdn());
        }

        return StringUtils.join(aliases, SEPARATOR_CHAR);
    }
}
