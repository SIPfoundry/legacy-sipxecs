/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.Collection;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class ReplicationTrigger implements ApplicationListener, DaoEventListener {
    protected static final Log LOG = LogFactory.getLog(ReplicationTrigger.class);

    private SipxReplicationContext m_replicationContext;

    private ParkOrbitContext m_parkOrbitContext;

    private SpeedDialManager m_speedDialManager;

    private SipxServiceManager m_sipxServiceManager;

    private boolean m_replicateOnStartup = true;

    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    public void setParkOrbitContext(ParkOrbitContext parkOrbitContext) {
        m_parkOrbitContext = parkOrbitContext;
    }

    public void setSpeedDialManager(SpeedDialManager speedDialManager) {
        m_speedDialManager = speedDialManager;
    }

    public void setSipxServiceManager(SipxServiceManager serviceManager) {
        m_sipxServiceManager = serviceManager;
    }

    public boolean isReplicateOnStartup() {
        return m_replicateOnStartup;
    }

    public void setReplicateOnStartup(boolean replicateOnStartup) {
        m_replicateOnStartup = replicateOnStartup;
    }

    public void onSave(Object entity) {
        onSaveOrDelete(entity);
    }

    public void onDelete(Object entity) {
        onSaveOrDelete(entity);
    }

    /**
     * Override ApplicationListener.onApplicationEvent so we can handle events. Perform data
     * replication every time the app starts up.
     */
    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof ApplicationInitializedEvent && isReplicateOnStartup()) {
            LOG.info("Replicating all data sets after application has initialized");
            m_replicationContext.generateAll();
            m_parkOrbitContext.activateParkOrbits();
            m_speedDialManager.activateResourceList();

            Collection<SipxService> allSipxServices = m_sipxServiceManager.getServiceDefinitions();
            for (SipxService sipxService : allSipxServices) {
                m_sipxServiceManager.replicateServiceConfig(sipxService);
            }
        }
    }

    void onSaveOrDelete(Object entity) {
        Class c = entity.getClass();
        if (Group.class.equals(c)) {
            Group group = (Group) entity;
            if ("user".equals(group.getResource())) {
                m_replicationContext.generate(DataSet.PERMISSION);
            }
        } else if (User.class.equals(c)) {
            m_replicationContext.generateAll();
        }
    }

    // testing only
    SipxReplicationContext getReplicationContext() {
        return m_replicationContext;
    }

    ParkOrbitContext getParkOrbitContext() {
        return m_parkOrbitContext;
    }

    SpeedDialManager getSpeedDialManager() {
        return m_speedDialManager;
    }
}
