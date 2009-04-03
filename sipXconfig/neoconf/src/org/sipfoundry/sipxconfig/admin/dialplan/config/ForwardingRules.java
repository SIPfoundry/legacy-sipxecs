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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.TemplateConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.DefaultSbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.SipxStatusService;

/**
 * Controls very initial SIP message routing from proxy based on SIP method and potentialy message
 * content.
 */
public class ForwardingRules extends TemplateConfigurationFile {
    private SbcManager m_sbcManager;

    private List<String> m_routes;
    private SipxServiceManager m_sipxServiceManager;
    private LocationsManager m_locationsManager;

    public void setSbcManager(SbcManager sbcManager) {
        m_sbcManager = sbcManager;
    }

    public void begin() {
        m_routes = new ArrayList<String>();
    }

    public void generate(IDialingRule rule) {
        m_routes.addAll(Arrays.asList(rule.getHostPatterns()));
    }

    public void end() {
    }

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        context.put("routes", m_routes);

        DefaultSbc sbc = m_sbcManager.loadDefaultSbc();
        context.put("sbc", sbc);

        if (sbc != null) {
            context.put("exportLocalIpAddress", !sbc.getRoutes().isEmpty());
        }

        context.put("auxSbcs", m_sbcManager.loadAuxSbcs());
        context.put("dollar", "$");

        SipxRegistrarService registrarService = (SipxRegistrarService) m_sipxServiceManager
                .getServiceByBeanId(SipxRegistrarService.BEAN_ID);

        // set required sipx services in context
        context.put("proxyService", m_sipxServiceManager.getServiceByBeanId(SipxProxyService.BEAN_ID));
        context.put("statusService", m_sipxServiceManager.getServiceByBeanId(SipxStatusService.BEAN_ID));
        context.put("registrarService", registrarService);

        context.put("location", location);

        List<Location> registrarLocations = m_locationsManager.getLocationsForService(registrarService);
        String registrarMappingLocation = registrarService.formatMappingLocation(location,
                registrarLocations.size() > 1);
        context.put("registrarMappingLocation", registrarMappingLocation);
        return context;
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

}
