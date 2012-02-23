/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.time;

import java.util.List;

import org.sipfoundry.sipxconfig.feature.GlobalFeature;

public interface TimeManager {
    static final GlobalFeature FEATURE = new GlobalFeature("ntpd");
    static final String CONTEXT_BEAN_NAME = "timeManager";

    void setSystemDate(String dateString);

    int getSystemTimeSettingType();

    String getNtpConfiguration();

    void setNtpConfiguration(String configuration);

    List<String> getNtpServers();

    void setNtpServers(List<String> ntpServers);

    void setSystemTimezone(String timezone);
}
