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
import static java.lang.String.format;
public class IpAddrWildCardSetting extends StringSetting {
    private static final String FIRST_OCTET = "(25[0-5]|2[0-4][0-9]|[01]?[0-9]{1,2})";
    private static final String OCTET = "(25[0-5]|2[0-4][0-9]|[01]?[0-9]{1,2}|\\*)";
    private static final String IP_ADDR = format("%1s(\\.%2$s){3}", FIRST_OCTET, OCTET);
    @Override
    public String getPattern() {
        return IP_ADDR;
    }
}
