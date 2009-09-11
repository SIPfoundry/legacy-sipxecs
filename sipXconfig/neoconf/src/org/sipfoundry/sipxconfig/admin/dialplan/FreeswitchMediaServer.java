/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Formatter;
import java.util.Map;

import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.service.SipxIvrService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;

/**
 * AutoAttendant and Voicemail build on top of the freeswitch media server.
 *
 * A better name for this class would be SipxIvrMediaServer.
 *
 * Voicemail URIs:
 *
 * <pre>
 * Deposit: sip:IVR@{vm server}:15060;mailbox={mailbox};action=deposit;locale={locale}
 * Retrieve: sip:IVR@{vm server}:15060;action=retrieve;locale={locale}
 * </pre>
 *
 */
public class FreeswitchMediaServer extends MediaServer {
    private static final String USER_PART = "IVR";
    private int m_port;
    private SipxServiceManager m_sipxServiceManager;

    public void setPort(int port) {
        m_port = port;
    }

    @Override
    public String getDigitStringForOperation(Operation operation, CallDigits userDigits) {
        return USER_PART;
    }

    @Override
    public String getHeaderParameterStringForOperation(Operation operation, CallDigits userDigits) {
        return null;
    }

    @Override
    protected String getUriParameterStringForOperation(Operation operation, CallDigits userDigits,
            Map<String, String> additionalParams) {
        StringBuilder params = new StringBuilder();
        Formatter f = new Formatter(params);

        if (operation == Operation.VoicemailDeposit) {
            f.format("mailbox={%s};action=deposit", userDigits.getName());
        } else {
            params.append("action=retrieve");
        }
        appendLocale(f);
        return params.toString();
    }

    @Override
    public String getName() {
        return "sipXivr";
    }

    @Override
    public PermissionName getPermissionName() {
        return PermissionName.FREESWITH_VOICEMAIL;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Override
    public String buildAttendantUrl(String attendantName) {
        StringBuilder params = new StringBuilder("action=autoattendant");
        Formatter f = new Formatter(params);
        f.format(";schedule_id=%s", attendantName);
        appendLocale(f);
        return MappingRule.buildUrl(USER_PART, getHostname(), params.toString(), null, null);
    }

    @Override
    public String getHostname() {
        SipxService service = m_sipxServiceManager.getServiceByBeanId(SipxIvrService.BEAN_ID);
        String hostAddress = service.getAddress();
        return String.format("%s:%d", hostAddress, m_port);
    }

    private void appendLocale(Formatter f) {
        String language = getLanguage();
        if (language != null) {
            f.format(";locale=%s", language);
        }
    }
}
