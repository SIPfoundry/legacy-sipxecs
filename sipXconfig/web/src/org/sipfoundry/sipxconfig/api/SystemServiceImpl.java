/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.api;

import java.rmi.RemoteException;
import java.util.Collection;

import org.sipfoundry.sipxconfig.domain.DomainManager;

public class SystemServiceImpl implements SystemService {
    private DomainManager m_domainManager;
    private ApiBeanBuilder m_builder = new SimpleBeanBuilder();

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public SystemInfo systemInfo() throws RemoteException {
        SystemInfo info = new SystemInfo();
        org.sipfoundry.sipxconfig.domain.Domain domain = m_domainManager.getDomain();
        Domain apiDomain = new Domain();
        ApiBeanUtil.toApiObject(m_builder, apiDomain, domain);
        info.setDomain(apiDomain);
        Collection<String> aliases = domain.getAliases();
        if (aliases != null) {
            apiDomain.setAliases(aliases.toArray(new String[aliases.size()]));
        }
        
        return info;
    }

}
