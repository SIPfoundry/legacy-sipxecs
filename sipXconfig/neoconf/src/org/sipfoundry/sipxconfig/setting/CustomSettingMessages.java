/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.Locale;
import java.util.ResourceBundle;

public final class CustomSettingMessages {
    public static final String INVALID_HOSTNAME_PATTERN = "invalid_hostname_pattern";
    public static final String INVALID_IPADDR_PATTERN = "invalid_ipaddr_pattern";
    public static final String INVALID_PHONEPADPIN_PATTERN = "invalid_phonepadpin_pattern";
    public static final String INVALID_USERNAME_SEQUENCE_PATTERN = "invalid_username_sequence_pattern";

    private static final String RESOURCE_BUNDLE = CustomSettingMessages.class.getName();

    private CustomSettingMessages() {
    }

    public static String getMessagePattern(String key, Locale locale) {
        return ResourceBundle.getBundle(RESOURCE_BUNDLE, locale).getString(key);
    }

}
