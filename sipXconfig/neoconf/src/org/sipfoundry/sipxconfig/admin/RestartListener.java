/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.Serializable;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.springframework.beans.factory.annotation.Required;

public class RestartListener implements Serializable, WaitingListener {
    private Map<Location, List<SipxService>> m_servicesMap;

    private SipxProcessContext m_sipxProcessContext;

    public void afterResponseSent() {
        restart();
    }

    public void restart() {
        m_sipxProcessContext.manageServices(m_servicesMap, SipxProcessContext.Command.RESTART);
    }

    public void setServicesMap(Map<Location, List<SipxService>> servicesMap) {
        m_servicesMap = servicesMap;
    }

    @Required
    public void setSipxProcessContext(SipxProcessContext sipxProcessContext) {
        m_sipxProcessContext = sipxProcessContext;
    }

}
