/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcRoutes;
import org.sipfoundry.sipxconfig.nattraversal.NatLocation;

public class NatTraversalConfiguration extends SipxServiceConfiguration {
    private SbcManager m_sbcManager;

    public void setSbcManager(SbcManager sbcManager) {
        m_sbcManager = sbcManager;
    }

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        SbcRoutes routes = m_sbcManager.getRoutes();
        context.put("routes", routes);

        NatLocation natLocation = location.getNat();
        context.put("natlocation", natLocation);
        context.put("service", getService(SipxRelayService.BEAN_ID));
        context.put("location", location);
        context.put("xmlRpcPort", Location.PROCESS_MONITOR_PORT);
        context.put("proxyService", getService(SipxProxyService.BEAN_ID));

        return context;
    }
}
