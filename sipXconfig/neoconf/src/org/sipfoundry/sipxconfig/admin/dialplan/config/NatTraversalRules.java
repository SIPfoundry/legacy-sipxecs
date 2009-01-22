/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.TemplateConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.DefaultSbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversal;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversalManager;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public class NatTraversalRules extends TemplateConfigurationFile {
    private SbcManager m_sbcManager;
    private NatTraversalManager m_natTraversalManager;
    private SipxServiceManager m_sipxServiceManager;

    public void setSbcManager(SbcManager sbcManager) {
        m_sbcManager = sbcManager;
    }

    public void setNatTraversalManager(NatTraversalManager natTraversalManager) {
        m_natTraversalManager = natTraversalManager;
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        DefaultSbc sbc = m_sbcManager.loadDefaultSbc();
        NatTraversal natTraversal = m_natTraversalManager.getNatTraversal();
        context.put("sbc", sbc);
        context.put("nattraversal", natTraversal);
        context.put("state", natTraversal.isEnabled() ? "enabled" : "disabled");
        context.put("behindnat", natTraversal.isBehindnat());
        context.put("location", location);
        SipxProxyService proxyService = (SipxProxyService) m_sipxServiceManager
                .getServiceByBeanId(SipxProxyService.BEAN_ID);
        context.put("proxyService", proxyService);

        return context;
    }
}
