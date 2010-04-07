/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.service;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.springframework.beans.factory.annotation.Required;

public class SipxImbotConfiguration extends SipxServiceConfiguration {
    private LocationsManager m_locationsManager;

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        SipxService service = getService(SipxImbotService.BEAN_ID);
        context.put("service", service);
        context.put("restService", getService(SipxRestService.BEAN_ID));
        context.put("sipxServiceManager", getSipxServiceManager());
        context.put("ivrService", getService(SipxIvrService.BEAN_ID));

        Location voicemailLocation = m_locationsManager.getLocationByBundle("voicemailBundle");
        if (voicemailLocation != null) {
            context.put("voicemailFqdn", voicemailLocation.getFqdn());
        }

        Location configLocation = m_locationsManager.getLocationByBundle("managementBundle");
        if (configLocation != null) {
            context.put("configFqdn", configLocation.getFqdn());
        }

        return context;
    }

    @Override
    public boolean isReplicable(Location location) {
        return getSipxServiceManager().isServiceInstalled(location.getId(), SipxImbotService.BEAN_ID)
                && getSipxServiceManager().getServiceByBeanId(SipxImbotService.BEAN_ID).isAvailable();
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
