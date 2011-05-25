/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.phone.snom;

public final class SnomM3Constants {
    // Snom m3 constans
    static final String NTP = "advanced/time/NETWORK_SNTP_SERVER";

    static final String DISPLAY_NAME = "Identity/_UA_DATA_DISP_NAME";
    static final String USER_NAME = "Identity/_SIP_UA_DATA_SIP_NAME";
    static final String PASSWORD = "Identity/_UA_DATA_AUTH_PASS";
    static final String AUTH_NAME = "Identity/_UA_DATA_AUTH_NAME";
    static final String ALIAS = "Identity/_SIP_UA_DATA_SIP_NAME_ALIAS";
    static final String MAILBOX = "Identity/_SIP_UA_DATA_VOICE_MAILBOX_NAME";
    static final String MAILBOX_NUMBER = "Identity/_SIP_UA_DATA_VOICE_MAILBOX_NUMBER";
    static final String DOMAIN = "Identity/_SIP_UA_DATA_DOMAIN";
    static final String OUTBOUND_PROXY = "Identity/_SIP_UA_DATA_PROXY_ADDR";

    private SnomM3Constants() {
        // do not instantiate
    }
}
