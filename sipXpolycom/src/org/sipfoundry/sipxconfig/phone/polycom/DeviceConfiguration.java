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

    public DeviceConfiguration(PolycomPhone device) {
        super(device, device.getTemplateDir() + "/device.cfg.vm");
    }
}
