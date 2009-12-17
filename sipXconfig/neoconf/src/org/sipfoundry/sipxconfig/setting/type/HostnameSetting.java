/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.setting.type;

import static java.lang.String.format;

public class HostnameSetting extends IpAddrSetting {
    private static final String DNS_LET_DIG = "[A-Za-z0-9]";
    private static final String DNS_LET_DIG_HYP = "[-A-Za-z0-9]";
    private static final String DNS_LABEL = format("(%s(%s*%1$s)?)", DNS_LET_DIG, DNS_LET_DIG_HYP);

    /**
     * DNS domain names have at least 2 labels, of which the last contains only letters and has at
     * least 2 chars. 'localhost' is also allowed..
     */
    private static final String DNS_DOMAIN_NAME = format("((%s\\.){1,}[A-Za-z]{2,}|localhost)", DNS_LABEL);

    @Override
    public String getPattern() {
        return DNS_DOMAIN_NAME + "|" + super.getPattern();
    }
}
