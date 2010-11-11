/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.conference;

import static java.util.Collections.singleton;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxImbotService;
import org.sipfoundry.sipxconfig.service.SipxIvrService;
import org.sipfoundry.sipxconfig.service.SipxRecordingService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class ConferenceBridgeProvisioningImpl extends HibernateDaoSupport implements ConferenceBridgeProvisioning {
    private SipxReplicationContext m_replicationContext;
    private ServiceConfigurator m_serviceConfigurator;
    private SipxServiceManager m_serviceManager;
    private SipxProcessContext m_processContext;

    public void deploy(Bridge bridge) {

        // bridge to deploy should always have service associated
        if (bridge.getService() == null) {
            return;
        }
        m_replicationContext.generate(DataSet.ALIAS);

        // only need to replicate files that do not require restart
        Location location = bridge.getLocation();
        SipxFreeswitchService freeswitchService = bridge.getFreeswitchService();
        m_serviceConfigurator.replicateServiceConfig(location, freeswitchService, true, false);
        m_processContext.markServicesForReload(singleton(freeswitchService));
        if (m_serviceManager.isServiceInstalled(SipxIvrService.BEAN_ID)) {
            SipxService ivrService = m_serviceManager.getServiceByBeanId(SipxIvrService.BEAN_ID);
            m_serviceConfigurator.replicateServiceConfig(ivrService, true);
        }
        if (m_serviceManager.isServiceInstalled(SipxRecordingService.BEAN_ID)) {
            SipxService ivrService = m_serviceManager.getServiceByBeanId(SipxRecordingService.BEAN_ID);
            m_serviceConfigurator.replicateServiceConfig(location, ivrService, true);
        }
        if (m_serviceManager.isServiceInstalled(SipxImbotService.BEAN_ID)) {
            SipxService imbotService = m_serviceManager.getServiceByBeanId(SipxImbotService.BEAN_ID);
            m_serviceConfigurator.replicateServiceConfig(location, imbotService, true);
        }
    }

    @Required
    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    @Required
    public void setServiceConfigurator(ServiceConfigurator serviceConfigurator) {
        m_serviceConfigurator = serviceConfigurator;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager serviceManager) {
        m_serviceManager = serviceManager;
    }

    @Required
    public void setSipxProcessContext(SipxProcessContext context) {
        m_processContext = context;
    }
}
