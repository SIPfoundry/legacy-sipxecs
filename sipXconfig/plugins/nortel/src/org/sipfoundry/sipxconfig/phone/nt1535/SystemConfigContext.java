/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.nt1535;

import org.sipfoundry.sipxconfig.device.ProfileContext;

/**
 * Responsible for generating sysconf_2890d_sip.cfg
 */
public class SystemConfigContext extends ProfileContext {
    private static final String SYSTEM_CONFIG_TEMPLATE = "nt1535/sysconf_2890d_sip.cfg.vm";


    public SystemConfigContext(NT1535Phone device) {
        super(device, SYSTEM_CONFIG_TEMPLATE);
    }

}
