/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.service.freeswitch;

import org.apache.commons.lang.StringUtils;
import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxServiceConfiguration;

public class ModulesConfiguration extends SipxServiceConfiguration {

    @Override
    public boolean isReplicable(Location location) {
        return getSipxServiceManager().isServiceInstalled(location.getId(), SipxFreeswitchService.BEAN_ID);
    }

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        SipxFreeswitchService service = (SipxFreeswitchService) getSipxServiceManager().
            getServiceByBeanId(SipxFreeswitchService.BEAN_ID);
        boolean g729 = StringUtils.contains(
                service.getSettings().getSetting(
                        SipxFreeswitchService.FREESWITCH_CODECS).getValue(), SipxFreeswitchService.G729);
        context.put("codecG729", g729);

        return context;
    }
}
