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

import org.sipfoundry.sipxconfig.setting.Setting;

public class UnmanagedService extends ConfiguredService {
    public static final String BEAN_ID = "unmanagedService";

    public static final ServiceDescriptor NTP = new ServiceDescriptor(BEAN_ID, "ntpService",
            "NTP");
    public static final ServiceDescriptor DNS = new ServiceDescriptor(BEAN_ID, "dnsService",
            "DNS");
    public static final ServiceDescriptor SYSLOG = new ServiceDescriptor(BEAN_ID,
            "syslogService", "Syslog");

    public UnmanagedService() {
        super(BEAN_ID);
    }

    public final ServiceDescriptor ntp() {
        return NTP;
    }

    public final ServiceDescriptor dns() {
        return DNS;
    }

    public final ServiceDescriptor syslog() {
        return SYSLOG;
    }

    @Override
    protected Setting loadSettings() {
        return null;
    }
}
