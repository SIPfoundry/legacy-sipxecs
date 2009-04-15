/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;


import org.sipfoundry.sipxconfig.device.ProfileContext;

/**
 * Responsible for generating ipmid.cfg
 */
public class DeviceConfiguration extends ProfileContext {
    private static final String DEVICE_TEMPLATE = PolycomPhone.TEMPLATE_DIR + "/device.cfg.vm";


    public DeviceConfiguration(PolycomPhone device) {
        super(device, DEVICE_TEMPLATE);
    }
}
