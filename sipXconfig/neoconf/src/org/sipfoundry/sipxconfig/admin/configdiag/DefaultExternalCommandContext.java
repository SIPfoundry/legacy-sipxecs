/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.admin.configdiag;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class DefaultExternalCommandContext implements ExternalCommandContext, Serializable {
    private static final String DOMAIN = "domain";
    private static final String HOSTNAME = "hostname";

    private String m_binDirectory;
    private final Map<String, String> m_argResolverMap;
    private DomainManager m_domainManager;
    private LocationsManager m_locationsManager;

    public DefaultExternalCommandContext() {
        m_argResolverMap = new HashMap<String, String>();
    }

    public String getBinDirectory() {
        return m_binDirectory;
    }

    public void setBinDirectory(String binDirectory) {
        m_binDirectory = binDirectory;
    }

    public String resolveArgumentString(String key) {
        initResolverMap();
        return m_argResolverMap.get(key);
    }

    private void initResolverMap() {
        if (m_argResolverMap.get(HOSTNAME) == null) {
            m_argResolverMap.put(HOSTNAME, m_locationsManager.getPrimaryLocation().getFqdn());
        }

        if (m_argResolverMap.get(DOMAIN) == null) {
            m_argResolverMap.put(DOMAIN, m_domainManager.getAuthorizationRealm());
        }
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
}
