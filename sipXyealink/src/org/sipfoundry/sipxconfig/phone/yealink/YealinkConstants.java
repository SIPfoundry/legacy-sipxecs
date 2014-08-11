/*
 * Copyright (c) 2013 SibTelCom, JSC (SIPLABS Communications). All rights reserved.
 * Contributed to SIPfoundry and eZuce, Inc. under a Contributor Agreement.
 *
 * Developed by Konstantin S. Vishnivetsky
 *
 * This library or application is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License (AGPL) as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option) any later version.
 *
 * This library or application is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License (AGPL) for
 * more details.
 *
 */

package org.sipfoundry.sipxconfig.phone.yealink;

public abstract class YealinkConstants {
    public static final String REMOTE_PHONEBOOK = "contacts/RemotePhoneBook/";

    public static final String MIME_TYPE_PLAIN = "text/plain";
    public static final String MIME_TYPE_XML = "text/xml";

    public static final String XML_CONTACT_DATA = "directory.xml";
    public static final String WEB_ITEMS_LEVEL = "webitemslevel.cfg";
    public static final String VENDOR = "Yealink";
    public static final String MAC_PREFIX = "001565";

    public static final String PKTYPES_V6X = "0,2,5,6,7,8,13,22,28,29,30,31,32,33,39,43,44,45,46,47";
    public static final String PKTYPES_V70 = "0,2,5,6,7,8,9,13,14,22,23,27,28,29,30,32,33,43,44,45,46,47,48,49,50";
    public static final String DKTYPES_V70 = "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,"
            + "27,34,35,38,39,40,41,42,45,46";
    public static final String DKTYPES_V71 = "0,11,12,13,14,15,16,17,18,22,23,24,25,27,34,35,38,40";

    // Line specific settings used in /etc/yealinkPhone/line_XX.xml
    public static final String USER_ID_V6X_SETTING = "account/UserName";
    public static final String USER_ID_V7X_SETTING = "basic/user_name";
    public static final String AUTH_ID_V6X_SETTING = "account/AuthName";
    public static final String AUTH_ID_V7X_SETTING = "basic/auth_name";
    public static final String DISPLAY_NAME_V6X_SETTING = "account/DisplayName";
    public static final String DISPLAY_NAME_V7X_SETTING = "basic/display_name";
    public static final String PASSWORD_V6X_SETTING = "account/password";
    public static final String PASSWORD_V7X_SETTING = "basic/password";
    public static final String REGISTRATION_SERVER_HOST_V6X_SETTING = "account/SIPServerHost";
    public static final String REGISTRATION_SERVER_HOST_V7X_SETTING = "basic/sip_server_host";
    public static final String REGISTRATION_SERVER_PORT_V6X_SETTING = "account/SIPServerPort";
    public static final String REGISTRATION_SERVER_PORT_V7X_SETTING = "basic/sip_server_port";
    public static final String OUTBOUND_HOST_V6X_SETTING = "account/OutboundHost";
    public static final String OUTBOUND_HOST_V7X_SETTING = "basic/outbound_host";
    public static final String OUTBOUND_PORT_V6X_SETTING = "account/OutboundPort";
    public static final String OUTBOUND_PORT_V7X_SETTING = "basic/outbound_port";
    public static final String BACKUP_OUTBOUND_HOST_V6X_SETTING = "account/BackOutboundHost";
    public static final String BACKUP_OUTBOUND_HOST_V7X_SETTING = "basic/backup_outbound_host";
    public static final String BACKUP_OUTBOUND_PORT_V6X_SETTING = "account/BackOutboundPort";
    public static final String BACKUP_OUTBOUND_PORT_V7X_SETTING = "basic/backup_outbound_port";
    public static final String VOICE_MAIL_NUMBER_V6X_SETTING = "Message/VoiceNumber";
    public static final String VOICE_MAIL_NUMBER_V7X_SETTING = "basic/voice_mail.number";

    // Phone specific settings used in /etc/yealinkPhone/phone_XX.xml
    public static final String DNS_SERVER1_V6X_SETTING = "network-wan/DNS/PrimaryDNS";
    public static final String DNS_SERVER1_V7X_SETTING = "network-wan/DNS/network.primary_dns";
    public static final String DNS_SERVER2_V6X_SETTING = "network-wan/DNS/SecondaryDNS";
    public static final String DNS_SERVER2_V7X_SETTING = "network-wan/DNS/network.secondary_dns";
    public static final String SYSLOG_SERVER_V6X_SETTING = "upgrade/SYSLOG/SyslogdIP";
    public static final String SYSLOG_SERVER_V7X_SETTING = "upgrade/SYSLOG/syslog.server";
    public static final String LOCAL_TIME_ZONE_V6X_SETTING = "preference/Time/TimeZone";
    public static final String LOCAL_TIME_ZONE_V7X_SETTING = "preference/local_time.time_zone";
    public static final String LOCAL_TIME_SERVER1_V6X_SETTING = "preference/Time/TimeServer1";
    public static final String LOCAL_TIME_SERVER1_V7X_SETTING = "preference/local_time.ntp_server1";
    public static final String LOCAL_TIME_SERVER2_V6X_SETTING = "preference/Time/TimeServer2";
    public static final String LOCAL_TIME_SERVER2_V7X_SETTING = "preference/local_time.ntp_server2";
    public static final String XML_BROWSER_SERVER_V6X_SETTING = "xml-browser/ServerIP";
    public static final String XML_BROWSER_SERVER_V7X_SETTING = "features/APISECURITY/push_xml.server";
    public static final String ACTION_URI_LIMIT_IP_V6X_SETTING = "Features/ActionURILimitIP";
    public static final String ACTION_URI_LIMIT_IP_V7X_SETTING = "features/APISECURITY/features.action_uri_limit_ip";
    public static final String DIRECT_CALL_PICKUP_CODE_V7X_SETTING = "features/CALLPICKUP/"
            + "features.pickup.direct_pickup_code";

    public static final String REMOTE_PHONEBOOK_0_NAME_V6X_SETTING = "contacts/RemotePhoneBook/0/Name";
    public static final String REMOTE_PHONEBOOK_0_URL_V6X_SETTING = "contacts/RemotePhoneBook/0/URL";
    public static final String REMOTE_PHONEBOOK_0_NAME_V7X_SETTING = REMOTE_PHONEBOOK + "remote_phonebook.data.1.name";
    public static final String REMOTE_PHONEBOOK_0_URL_V7X_SETTING = REMOTE_PHONEBOOK + "remote_phonebook.data.1.url";

    public static final String REMOTE_PHONEBOOK_1_NAME_V6X_SETTING = "contacts/RemotePhoneBook/1/Name";
    public static final String REMOTE_PHONEBOOK_1_URL_V6X_SETTING = "contacts/RemotePhoneBook/1/URL";
    public static final String REMOTE_PHONEBOOK_1_NAME_V7X_SETTING = REMOTE_PHONEBOOK + "remote_phonebook.data.2.name";
    public static final String REMOTE_PHONEBOOK_1_URL_V7X_SETTING = REMOTE_PHONEBOOK  + "remote_phonebook.data.2.url";

    public static final String REMOTE_PHONEBOOK_2_NAME_V6X_SETTING = "contacts/RemotePhoneBook/2/Name";
    public static final String REMOTE_PHONEBOOK_2_URL_V6X_SETTING = "contacts/RemotePhoneBook/2/URL";
    public static final String REMOTE_PHONEBOOK_2_NAME_V7X_SETTING = REMOTE_PHONEBOOK + "remote_phonebook.data.3.name";
    public static final String REMOTE_PHONEBOOK_2_URL_V7X_SETTING = REMOTE_PHONEBOOK + "remote_phonebook.data.3.url";

    public static final String REMOTE_PHONEBOOK_3_NAME_V6X_SETTING = "contacts/RemotePhoneBook/3/Name";
    public static final String REMOTE_PHONEBOOK_3_URL_V6X_SETTING = "contacts/RemotePhoneBook/3/URL";
    public static final String REMOTE_PHONEBOOK_3_NAME_V7X_SETTING = REMOTE_PHONEBOOK + "remote_phonebook.data.4.name";
    public static final String REMOTE_PHONEBOOK_3_URL_V7X_SETTING = REMOTE_PHONEBOOK + "remote_phonebook.data.4.url";

    public static final String REMOTE_PHONEBOOK_4_NAME_V6X_SETTING = "contacts/RemotePhoneBook/4/Name";
    public static final String REMOTE_PHONEBOOK_4_URL_V6X_SETTING = "contacts/RemotePhoneBook/4/URL";
    public static final String REMOTE_PHONEBOOK_4_NAME_V7X_SETTING = REMOTE_PHONEBOOK + "remote_phonebook.data.5.name";
    public static final String REMOTE_PHONEBOOK_4_URL_V7X_SETTING = REMOTE_PHONEBOOK + "remote_phonebook.data.5.url";

    public static final String FIRMWARE_SERVER_ADDRESS_SETTING = "upgrade/firmware/server_ip";
    public static final String FIRMWARE_URL_V6X_SETTING = "upgrade/firmware/url";
    public static final String FIRMWARE_URL_V7X_SETTING = "upgrade/firmware/firmware.url";
    public static final String FIRMWARE_HTTP_URL_SETTING = "upgrade/firmware/http_url";
    public static final String FIRMWARE_NAME_SETTING = "upgrade/firmware/firmware_name";
    public static final String AUTOPROVISIONING_SERVER_URL_V6X_SETTING = "upgrade/autoprovision/strServerURL";
    public static final String AUTOPROVISIONING_SERVER_URL_V7X_SETTING = "upgrade/autoprovision/"
            + "auto_provision.server.url";
    public static final String AUTOPROVISIONING_SERVER_ADDRESS_V6X_SETTING = "upgrade/autoprovision/server_address";
    public static final String ADVANCED_MUSIC_SERVER_URI_V6X_SETTING = "account/MusicServerUri";
    public static final String ADVANCED_MUSIC_SERVER_URI_V7X_SETTING = "advanced/music_server_uri";
    // T2X except T20
    public static final String LOGO_FILE_NAME_V6X_SETTING = "upgrade/Logo/server_address";
    public static final String LOGO_FILE_NAME_V7X_SETTING = "features/GENERAL/lcd_logo.url";

    public static final String DIAL_NOW_URL_V7X_SETTING = "downloads/dialplan_dialnow.url";

    public static final String FEATURE_HDSOUND = "hasHDSound";
    public static final String FEATURE_PHONEBOOK = "hasPhoneBook";
    public static final String FEATURE_RINGTONES = "hasRingTones";
    public static final String FEATURE_WALLPAPERS = "hasWallPapers";
    public static final String FEATURE_SCREENSAVERS = "hasScreenSavers";
    public static final String FEATURE_LANGUAGES = "hasLanguages";

    // LDAP
    public static final String LDAP_ENABLE = "contacts/LDAP/ldap.enable";
    public static final String LDAP_HOST = "contacts/LDAP/ldap.host";
    public static final String LDAP_PORT = "contacts/LDAP/ldap.port";
    public static final String LDAP_USER = "contacts/LDAP/ldap.user";
    public static final String LDAP_PASSWORD = "contacts/LDAP/ldap.password";
    public static final String LDAP_BASE = "contacts/LDAP/ldap.base";
}
