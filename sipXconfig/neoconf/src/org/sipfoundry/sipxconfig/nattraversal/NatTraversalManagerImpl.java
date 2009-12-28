/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.nattraversal;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;

public class NatTraversalManagerImpl implements NatTraversalManager, DaoEventListener {

    private SipxServiceManager m_sipxServiceManager;

    private ServiceConfigurator m_serviceConfigurator;

    public void store(NatTraversal natTraversal) {
        natTraversal.store(m_sipxServiceManager);
        natTraversal.activate(m_serviceConfigurator);
    }

    public NatTraversal getNatTraversal() {
        return new NatTraversal(m_sipxServiceManager);
    }

    public void activateNatLocation(Location location) {
        getNatTraversal().activateOnLocation(location, m_serviceConfigurator);
    }

    public void onSave(Object entity) {
        if (entity instanceof NatLocation) {
            activateNatLocation(((NatLocation) entity).getLocation());
        }
    }

    public void onDelete(Object entity) {
        if (entity instanceof Location) {
            activateNatLocation((Location) entity);
        }
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Required
    public void setServiceConfigurator(ServiceConfigurator serviceConfigurator) {
        m_serviceConfigurator = serviceConfigurator;
    }
}
