/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxconfig.phone.snom;

final class SnomConstants {

    static final String USER_HOST = "line/user_host";
    // suspiciously, no registration server port?

    static final String MAILBOX = "line/user_mailbox";
    static final String OUTBOUND_PROXY = "line/user_outbound";
    static final String USER_NAME = "line/user_name";
    static final String PASSWORD = "line/user_pass";
    static final String AUTH_NAME = "line/user_pname";
    static final String DISPLAY_NAME = "line/user_realname";

    static final String USER_MOH = "sip/user_moh";
    static final String EV_LIST_SUBSCR = "sip/user_event_list_subscription";
    static final String EV_LIST_URI = "sip/user_event_list_uri";

    static final String TIMEZONE_SETTING = "network/utc_offset";
    static final String CONFIG_URL = "update/setting_server";
    static final String DST_SETTING = "network/dst";

    static final String NTP = "Basic_Network_Settings/ntp_server";
    static final String SYSLOG_SERVER = "Advanced_Network_Settings/syslog_server";

    private SnomConstants() {
        // do not instantiate
    }
}
