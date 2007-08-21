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

import java.util.Map;

import org.apache.commons.lang.StringUtils;

public class SipXMediaServer extends MediaServer {

    private static final String NAME = "SipXMediaServer";
    private static final String MEDIA_SERVER_HOSTNAME = "{mediaserver}";
    private static final String VOICEMAIL_DEPOSIT_PATTERN = 
        "{voicemail}/cgi-bin/voicemail/mediaserver.cgi?action=deposit&mailbox={%s-escaped}";
    private static final String VOICEMAIL_RETRIEVE_PATTERN = 
        "{voicemail}/cgi-bin/voicemail/mediaserver.cgi?action=retrieve";
    private static final String AUTOATTENDANT_PATTERN = 
        "{voicemail}/cgi-bin/voicemail/mediaserver.cgi?action=autoattendant&name=%s";
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
        StringBuffer paramStringBuffer = new StringBuffer();

        switch (operation) {
        case Autoattendant:
            String nameKey = "name";
            if (additionalParams == null || !additionalParams.containsKey(nameKey)) {
                throw new IllegalArgumentException("additionalParams map is missing name key");
            }
            String name = additionalParams.get(nameKey);
            paramStringBuffer.append(String.format(AUTOATTENDANT_PATTERN, name));
            break;
        case VoicemailDeposit:
            paramStringBuffer.append(String.format(VOICEMAIL_DEPOSIT_PATTERN, digits.getName()));
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

        return "voicexml=" + encodeParams(paramStringBuffer.toString(), ENCODE_EXCLUDES);
    }

    public String getName() {
        return NAME;
    }

    public String getDigitStringForOperation(Operation operation, CallDigits userDigits) {
        return '{' + userDigits.getName() + '}';
    }
    
    /**
     * Override the superclass implementation to always return the String 
     * "{mediaserver}"
     * @return The String "{mediaserver}"
     */
    public String getHostname() {
        return MEDIA_SERVER_HOSTNAME;
    }
}
