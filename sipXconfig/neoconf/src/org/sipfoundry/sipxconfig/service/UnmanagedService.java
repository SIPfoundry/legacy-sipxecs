/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.address.AddressType;

public interface UnmanagedService {
    //public static final GlobalFeature FEATURE = new GlobalFeature("unmanagedServices");
    public static final AddressType SYSLOG = new AddressType("syslog", 514);
    public static final AddressType DNS = new AddressType("dns", 53);
    public static final AddressType SSH = new AddressType("ssh", 22);

    public UnmanagedServiceSettings getSettings();

    public void saveSettings(UnmanagedServiceSettings settings);
}
