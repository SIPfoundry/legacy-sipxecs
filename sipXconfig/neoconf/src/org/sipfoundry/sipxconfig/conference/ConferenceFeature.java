/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;

public class ConferenceFeature implements FeatureListener, FeatureProvider {
    public static final LocationFeature CONFERENCE = new LocationFeature("conference");
    private ConferenceBridgeContext m_conferengeBridgeContext;

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(CONFERENCE);
    }

    @Override
    public void enableLocationFeature(FeatureEvent event, LocationFeature feature, Location location) {
        if (feature.equals(CONFERENCE)) {
            Bridge bridge = m_conferengeBridgeContext.getBridgeByServer(location.getFqdn());
            if (bridge == null && event == FeatureEvent.PRE_ENABLE) {
                bridge = m_conferengeBridgeContext.newBridge();
                bridge.setService(location.getService(SipxFreeswitchService.BEAN_ID));
                m_conferengeBridgeContext.store(bridge);
            }
            if (bridge != null && event == FeatureEvent.POST_ENABLE) {
                deploy(bridge);
            }
            if (bridge != null && event == FeatureEvent.PRE_DISABLE) {
                m_conferengeBridgeContext.removeConferences(Collections.singleton(bridge.getId()));
                deploy(bridge);
            }
        }
    }

    public void deploy(Bridge bridge) {
//
//        // bridge to deploy should always have service associated
//        if (bridge.getService() == null) {
//            return;
//        }
//        // need to replicate all conferences that are on the bridge
//        for (Conference conf : bridge.getConferences()) {
//            m_replicationContext.generate(conf);
//        }
//
//        // only need to replicate files that do not require restart
//        Location location = bridge.getLocation();
//        SipxFreeswitchService freeswitchService = bridge.getFreeswitchService();
//        // we need to flush here. sipX_context.xml replication needs up-to-date info in sql tables
//        getHibernateTemplate().flush();
//        m_serviceConfigurator.replicateServiceConfig(location, freeswitchService, true, false);
//        m_processContext.markServicesForReload(singleton(freeswitchService));
//        if (m_serviceManager.isServiceInstalled(SipxIvrService.BEAN_ID)) {
//            SipxService ivrService = m_serviceManager.getServiceByBeanId(SipxIvrService.BEAN_ID);
//            m_serviceConfigurator.replicateServiceConfig(ivrService, true);
//        }
//        if (m_serviceManager.isServiceInstalled(SipxRecordingService.BEAN_ID)) {
//            SipxService ivrService = m_serviceManager.getServiceByBeanId(SipxRecordingService.BEAN_ID);
//            m_serviceConfigurator.replicateServiceConfig(location, ivrService, true);
//        }
//        if (m_serviceManager.isServiceInstalled(SipxImbotService.BEAN_ID)) {
//            SipxService imbotService = m_serviceManager.getServiceByBeanId(SipxImbotService.BEAN_ID);
//            m_serviceConfigurator.replicateServiceConfig(location, imbotService, true);
//        }
    }

    @Override
    public void enableGlobalFeature(FeatureEvent event, GlobalFeature feature) {
    }

    public void setConferengeBridgeContext(ConferenceBridgeContext conferengeBridgeContext) {
        m_conferengeBridgeContext = conferengeBridgeContext;
    }
}
