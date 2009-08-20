/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.springframework.beans.factory.annotation.Required;

public class SipxPresenceConfiguration extends SipxServiceConfiguration {

    private AcdContext m_acdContext;

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        SipxService service = getService(SipxPresenceService.BEAN_ID);
        addLocationIdSuffix(service, location);
        context.put("settings", service.getSettings().getSetting("presence-config"));
        context.put("presenceService", service);
        return context;
    }

    @Override
    public boolean isReplicable(Location location) {
        return getSipxServiceManager().isServiceInstalled(location.getId(), SipxPresenceService.BEAN_ID);
    }

    private void addLocationIdSuffix(SipxService service, Location location) {
        if (1 < m_acdContext.getServers().size()) {
            String signInCode = service.getSettingValue(SipxPresenceService.PRESENCE_SIGN_IN_CODE);
            String signOutCode = service.getSettingValue(SipxPresenceService.PRESENCE_SIGN_OUT_CODE);
            signInCode += String.valueOf(location.getId());
            signOutCode += String.valueOf(location.getId());
            service.setSettingTypedValue(SipxPresenceService.PRESENCE_SIGN_IN_CODE, signInCode);
            service.setSettingTypedValue(SipxPresenceService.PRESENCE_SIGN_OUT_CODE, signOutCode);
        }
    }

    @Required
    public void setAcdContext(AcdContext acdContext) {
        m_acdContext = acdContext;
    }
}
