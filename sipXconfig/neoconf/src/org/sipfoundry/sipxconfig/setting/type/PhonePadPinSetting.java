/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.setting.type;

/**
 * SettingType to represent a pin associated with a phone key pad. The allowed characters are
 * numbers, # and * keys, and usually used for conference participation code.
 */
public class PhonePadPinSetting extends StringSetting {
    private static final String PHONE_PAD = "[\\d#*]+";

    @Override
    public String getPattern() {
        return PHONE_PAD;
    }

}
