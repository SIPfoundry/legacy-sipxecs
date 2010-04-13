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

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class SystemServiceImpl implements SystemService {
    private DomainManager m_domainManager;
    private CoreContext m_coreContext;
    private ApiBeanBuilder m_builder = new SystemInfoBuilder();

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public SystemInfo systemInfo() throws RemoteException {
        SystemInfo info = new SystemInfo();
        org.sipfoundry.sipxconfig.domain.Domain domain = m_domainManager.getDomain();
        Domain apiDomain = new Domain();
        ApiBeanUtil.toApiObject(m_builder, apiDomain, domain);
        info.setDomain(apiDomain);
        apiDomain.setRealm(m_coreContext.getAuthorizationRealm());

        return info;
    }

}
