/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

public interface TimeZoneManager {
    static final String CONTEXT_BEAN_NAME = "timeZoneManager";

    DeviceTimeZone getDeviceTimeZone();

    void saveDefault();

    void setDeviceTimeZone(DeviceTimeZone deviceTimeZone);
}
