/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxIvrService;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;

public abstract class EagerDialPlanActivationManager implements BeanFactoryAware, DialPlanActivationManager {
    private SipxReplicationContext m_sipxReplicationContext;

    private SbcDeviceManager m_sbcDeviceManager;

    private ProfileManager m_sbcProfileManager;

    private SipxServiceManager m_sipxServiceManager;

    private SipxProcessContext m_sipxProcessContext;

    private BeanFactory m_beanFactory;

    private DialPlanContext m_dialPlanContext;

    private LocationsManager m_locationsManager;

    /* delayed injection - working around circular reference */
    public abstract ServiceConfigurator getServiceConfigurator();

    /* delayed injection - working around circular reference */
    public abstract GatewayContext getGatewayContext();

    /* delayed injection - working around circular reference */
    public abstract ProfileManager getGatewayProfileManager();

    public void replicateDialPlan(boolean restartSbcDevices) {
        for (Location location : m_locationsManager.getLocations()) {
            ConfigGenerator generator = generateDialPlan();
            generator.activate(location, m_sipxReplicationContext);
        }
        SipxService sipxIvrService = m_sipxServiceManager.getServiceByBeanId(SipxIvrService.BEAN_ID);
        getServiceConfigurator().replicateServiceConfig(sipxIvrService, true);

        pushAffectedProfiles(restartSbcDevices);
        notifyOnDialPlanGeneration();
    }

    public void replicateIfNeeded() {
        // empty: conditional activation only makes sense for a lazy implementation
    }

    ConfigGenerator generateDialPlan() {
        ConfigGenerator generator = createConfigGenerator();
        generator.generate(m_dialPlanContext);
        return generator;
    }

    private ConfigGenerator createConfigGenerator() {
        ConfigGenerator generator = (ConfigGenerator) m_beanFactory.getBean(ConfigGenerator.BEAN_NAME,
                ConfigGenerator.class);
        return generator;
    }

    /**
     * Notify the world we are done with activating dial plan
     */
    private void notifyOnDialPlanGeneration() {
        m_sipxReplicationContext.publishEvent(new DialPlanActivatedEvent(this));
        m_sipxProcessContext.markDialPlanRelatedServicesForRestart(SipxProxyService.BEAN_ID,
                SipxRegistrarService.BEAN_ID);
    }

    /**
     * Push gateway and SBC devices generation when activating dial plan
     *
     * @param restartSbcDevices if false it does not try to restart affected devices
     */
    private void pushAffectedProfiles(boolean restartSbcDevices) {
        Collection<Integer> gatewayIds = getGatewayContext().getAllGatewayIds();
        getGatewayProfileManager().generateProfiles(gatewayIds, true, null);
        Collection<Integer> sbcIds = m_sbcDeviceManager.getAllSbcDeviceIds();
        m_sbcProfileManager.generateProfiles(sbcIds, restartSbcDevices, null);
    }

    @Required
    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }

    @Required
    public void setSbcProfileManager(ProfileManager sbcProfileManager) {
        m_sbcProfileManager = sbcProfileManager;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Required
    public void setSipxProcessContext(SipxProcessContext sipxProcessContext) {
        m_sipxProcessContext = sipxProcessContext;
    }

    @Required
    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    @Required
    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }
}
