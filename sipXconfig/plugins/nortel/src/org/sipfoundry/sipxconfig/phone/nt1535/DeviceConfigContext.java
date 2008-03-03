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
public class DeviceConfigContext extends ProfileContext {
    private static final String DEVICE_TEMPLATE = "nt1535/mac.cfg.vm";


    public DeviceConfigContext(NT1535Phone device) {
        super(device, DEVICE_TEMPLATE);
    }

}
