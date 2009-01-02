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
import java.util.TreeMap;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.service.SipxMediaService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public class SipXMediaServer extends MediaServer {

    private static final String NAME = "SipXMediaServer";
    private static final String VOICEMAIL_DEPOSIT_PATTERN =
        "/cgi-bin/voicemail/mediaserver.cgi?action=deposit&mailbox={%s-escaped}";
    private static final String VOICEMAIL_RETRIEVE_PATTERN = "/cgi-bin/voicemail/mediaserver.cgi?action=retrieve";
    private static final String AUTOATTENDANT_PATTERN = "/cgi-bin/voicemail/mediaserver.cgi?action=autoattendant";

    private SipxServiceManager m_sipxServiceManager;

    @Override
    public String getHeaderParameterStringForOperation(Operation operation, CallDigits digits) {
        return null;
    }

    @Override
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
            paramStringBuffer.append(getVoicemailServer() + AUTOATTENDANT_PATTERN);
            break;
        case VoicemailDeposit:
            formatter.format(getVoicemailServer() + VOICEMAIL_DEPOSIT_PATTERN, digits.getName());
            break;
        case VoicemailRetrieve:
            paramStringBuffer.append(getVoicemailServer() + VOICEMAIL_RETRIEVE_PATTERN);
            break;
        default:
            return StringUtils.EMPTY;
        }

        Map<String, String> params = new TreeMap<String, String>();
        String locale = getLanguage();
        if (StringUtils.isNotBlank(locale)) {
            params.put("lang", locale);
        }
        if (null != additionalParams) {
            params.putAll(additionalParams);
        }

        for (Map.Entry<String, String> entry : params.entrySet()) {
            formatter.format("&%s=%s", entry.getKey(), entry.getValue());
        }
        return "voicexml=" + encodeParams(paramStringBuffer.toString(), ENCODE_EXCLUDES);
    }

    @Override
    public String getName() {
        return NAME;
    }

    @Override
    public String getDigitStringForOperation(Operation operation, CallDigits userDigits) {
        return '{' + userDigits.getName() + '}';
    }

    /**
     * Override the superclass implementation to return the media server value from the media
     * service"
     *
     * @return The media server as defined by SipxRegistrarService
     */
    @Override
    public String getHostname() {
        SipxMediaService mediaService = (SipxMediaService) m_sipxServiceManager
                .getServiceByBeanId(SipxMediaService.BEAN_ID);
        return mediaService.getMediaServer();
    }

    @Override
    public PermissionName getPermissionName() {
        return PermissionName.SIPX_VOICEMAIL;
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    private String getVoicemailServer() {
        SipxMediaService mediaService = (SipxMediaService) m_sipxServiceManager
                .getServiceByBeanId(SipxMediaService.BEAN_ID);
        return mediaService.getVoicemailServer();
    }
}
