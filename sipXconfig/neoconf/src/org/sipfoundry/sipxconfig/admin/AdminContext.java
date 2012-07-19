/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alarm.AlarmDefinition;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface AdminContext {
    public static final String URL_FORMAT = "http://%s:%d";
    public static final LocationFeature FEATURE = new LocationFeature("admin");
    public static final AddressType HTTP_ADDRESS = new AddressType("adminApi", URL_FORMAT);
    public static final AddressType HTTP_ADDRESS_AUTH = new AddressType("adminApiAuth", URL_FORMAT);
    public static final AddressType SIPXCDR_DB_ADDRESS = new AddressType("sipxcdr", "jdbc:postgresql://%s/SIPXCDR");
    public static final AlarmDefinition ALARM_LOGIN_FAILED = new AlarmDefinition("LOGIN_FAILED", 3);
    public static final String ARCHIVE = "configuration.tar.gz";
    final String CONTEXT_BEAN_NAME = "adminContext";

    public AdminSettings getSettings();

    public void saveSettings(AdminSettings settings);
}
