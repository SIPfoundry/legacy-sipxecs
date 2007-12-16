/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.paging;

import java.util.Arrays;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.Process;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;

public abstract class PagingProvisioningContextImpl implements PagingProvisioningContext {

    private PagingContext m_pagingContext;

    private SipxReplicationContext m_replicationContext;

    private SipxProcessContext m_processContext;

    private DomainManager m_domainManager;

    private String m_audioDirectory;

    protected abstract PagingConfiguration createPagingConfiguration();

    private void replicatePagingConfig() {
        PagingServer pagingServer = m_pagingContext.getPagingServer();
        List<PagingGroup> pagingGroups = m_pagingContext.getPagingGroups();

        PagingConfiguration pagingConfiguration = createPagingConfiguration();
        String domainName = m_domainManager.getDomain().getName();
        pagingConfiguration.generate(pagingServer, pagingGroups, m_audioDirectory, domainName);
        m_replicationContext.replicate(pagingConfiguration);
        m_replicationContext.publishEvent(new PagingServerActivatedEvent(pagingConfiguration));
    }

    /**
     * Write new configuration and restart paging server
     */
    public void deploy() {
        replicatePagingConfig();
        Process service = m_processContext.getProcess(SipxProcessModel.ProcessName.PAGE_SERVER);
        m_processContext.restartOnEvent(Arrays.asList(service), PagingServerActivatedEvent.class);
    }

    @Required
    public void setPagingContext(PagingContext pagingContext) {
        m_pagingContext = pagingContext;
    }

    @Required
    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    @Required
    public void setProcessContext(SipxProcessContext processContext) {
        m_processContext = processContext;
    }

    @Required
    public void setAudioDirectory(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }
    
    public String getAudioDirectory() {
        return m_audioDirectory;
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
}
