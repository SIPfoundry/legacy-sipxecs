/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Formatter;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.permission.PermissionName;

public class SipXMediaServer extends MediaServer {

    private static final String NAME = "SipXMediaServer";
    private static final String MEDIA_SERVER_HOSTNAME = "{mediaserver}";
    private static final String VOICEMAIL_DEPOSIT_PATTERN =
        "{voicemail}/cgi-bin/voicemail/mediaserver.cgi?action=deposit&mailbox={%s-escaped}";
    private static final String VOICEMAIL_RETRIEVE_PATTERN =
        "{voicemail}/cgi-bin/voicemail/mediaserver.cgi?action=retrieve";
    private static final String AUTOATTENDANT_PATTERN =
        "{voicemail}/cgi-bin/voicemail/mediaserver.cgi?action=autoattendant";
    private static final String SOS_PATTERN = "{voicemail}/cgi-bin/voicemail/mediaserver.cgi?action=sos";

    /**
     * Creates a new instance of SipXMediaServer
     */
    public SipXMediaServer() {
        setHostname(MEDIA_SERVER_HOSTNAME);
    }

    public String getHeaderParameterStringForOperation(Operation operation, CallDigits digits,
            Map<String, String> additionalParams) {
        return null;
    }

    public String getUriParameterStringForOperation(Operation operation, CallDigits digits,
            Map<String, String> additionalParams) {
        StringBuilder paramStringBuffer = new StringBuilder();
        Formatter formatter = new Formatter(paramStringBuffer);

        switch (operation) {
        case Autoattendant:
            String nameKey = "name";
            if (additionalParams == null || !additionalParams.containsKey(nameKey)) {
                throw new IllegalArgumentException("additionalParams map is missing name key");
            }
            paramStringBuffer.append(AUTOATTENDANT_PATTERN);
            break;
        case VoicemailDeposit:
            formatter.format(VOICEMAIL_DEPOSIT_PATTERN, digits.getName());
            break;
        case VoicemailRetrieve:
            paramStringBuffer.append(VOICEMAIL_RETRIEVE_PATTERN);
            break;
        case SOS:
            paramStringBuffer.append(SOS_PATTERN);
            break;
        default:
            return StringUtils.EMPTY;
        }
        if (null != additionalParams) {
            for (Map.Entry<String, String> entry : additionalParams.entrySet()) {
                formatter.format("&%s=%s", entry.getKey(), entry.getValue());
            }
        }
        return "voicexml=" + encodeParams(paramStringBuffer.toString(), ENCODE_EXCLUDES);
    }

    public String getName() {
        return NAME;
    }

    public String getDigitStringForOperation(Operation operation, CallDigits userDigits) {
        return '{' + userDigits.getName() + '}';
    }

    /**
     * Override the superclass implementation to always return the String "{mediaserver}"
     * 
     * @return The String "{mediaserver}"
     */
    public String getHostname() {
        return MEDIA_SERVER_HOSTNAME;
    }

    public PermissionName getPermissionName() {
        return PermissionName.SIPX_VOICEMAIL;
    }

    protected void addLang(Map<String, String> params, String locale) {
        if (StringUtils.isNotBlank(locale)) {
            params.put("lang", locale);
        }
    }
}
