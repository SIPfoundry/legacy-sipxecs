/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.configdiag;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class DefaultExternalCommandContext implements ExternalCommandContext, Serializable {
    private static final Log LOG = LogFactory.getLog(DefaultExternalCommandContext.class);
    private String m_binDirectory;
    private Map<String, String> m_argResolverMap;

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
        return m_argResolverMap.get(key);
    }
    
    public void setLocationsManager(LocationsManager locationsManager) {
        if (locationsManager.getPrimaryLocation() != null) {
            m_argResolverMap.put("hostname", locationsManager.getPrimaryLocation().getFqdn());
        }
    }
    
    public void setDomainManager(DomainManager domainManager) {
        try {
            m_argResolverMap.put("domain", domainManager.getAuthorizationRealm());
        } catch (DomainManager.DomainNotInitializedException e) {
            LOG.warn("Unable to get authorization realm; domain not initialized", e);
        }
    }
}
