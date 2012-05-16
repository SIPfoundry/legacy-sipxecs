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

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.LinkedHashSet;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.cfgmgt.CfengineModuleConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;

public class DomainConfiguration implements ConfigProvider {
    private LocationsManager m_locationsManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(DomainManager.FEATURE)) {
            return;
        }
        DomainManager domainManager = manager.getDomainManager();
        Domain domain = domainManager.getDomain();
        String fqdn = m_locationsManager.getPrimaryLocation().getFqdn();
        String lang = manager.getDomainManager().getExistingLocalization().getLanguage();
        File gdir = manager.getGlobalDataDirectory();
        Writer wtr1 = new FileWriter(new File(gdir, "domain-config.part"));
        try {
            writeDomainConfigPart(wtr1, domain, fqdn);
        } finally {
            IOUtils.closeQuietly(wtr1);
        }

        Writer wtr2 = new FileWriter(new File(gdir, "domain.cfdat"));
        try {
            write(wtr2, domain, lang);
        } finally {
            IOUtils.closeQuietly(wtr2);
        }
    }

    void write(Writer wtr, Domain domain, String lang) throws IOException {
        CfengineModuleConfiguration config = new CfengineModuleConfiguration(wtr);
        config.write("domain", domain.getName());
        config.write("realm", domain.getSipRealm());
        config.write("secret", domain.getSharedSecret());
        config.write("lang", lang);
    }

    void writeDomainConfigPart(Writer wtr, Domain domain, String fqdn) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.colonSeparated(wtr);
        config.write("SIP_DOMAIN_ALIASES", getDomainAliases(domain));
        config.write("CONFIG_HOSTS", fqdn);
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
            String fqdn = location.getFqdn();
            if (!fqdn.equalsIgnoreCase(domain.getName())) {
                aliases.add(fqdn);
            }
        }

        return StringUtils.join(aliases, ' ');
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
