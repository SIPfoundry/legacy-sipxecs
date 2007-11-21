/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin;

import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivatedEvent;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.common.InitTaskListener;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;

public class FirstRunTask extends InitTaskListener {
    private static final Log LOG = LogFactory.getLog(FirstRunTask.class);

    private DomainManager m_domainManager;

    private DialPlanContext m_dialPlanContext;

    private SipxProcessContext m_processContext;

    public void onInitTask(String task) {
        LOG.info("Executing first run tasks...");
        m_domainManager.initialize();
        m_domainManager.replicateDomainConfig();
        m_dialPlanContext.activateDialPlan();
        List restartable = m_processContext.getRestartable();
        m_processContext.restartOnEvent(restartable, DialPlanActivatedEvent.class);
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Required
    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    @Required
    public void setProcessContext(SipxProcessContext processContext) {
        m_processContext = processContext;
    }
}
