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

public class FreeswitchMediaServer extends MediaServer {
    private int m_port;

    public void setPort(int port) {
        m_port = port;
    }

    @Override
    public String getDigitStringForOperation(Operation operation, CallDigits userDigits) {
        throw new NotImplementedException();
    }

    @Override
    public String getHeaderParameterStringForOperation(Operation operation, CallDigits userDigits) {
        throw new NotImplementedException();
    }

    @Override
    public String getName() {
        return "sipXivr";
    }

    @Override
    public PermissionName getPermissionName() {
        return null;
    }

    @Override
    public String buildAttendantUrl(String attendantName) {
        StringBuilder params = new StringBuilder("action=autoattendant");
        Formatter f = new Formatter(params);
        f.format(";schedule_id=%s", attendantName);
        String language = getLanguage();
        if (language != null) {
            f.format(";locale=%s", language);
        }
        String host = String.format("%s:%d", getHostname(), m_port);
        return MappingRule.buildUrl("IVR", host, params.toString(), null, null);
    }

    @Override
    public String buildVoicemailDepositUrl(String fieldParams) {
        throw new NotImplementedException();
    }

    @Override
    public String buildVoicemailRetrieveUrl() {
        throw new NotImplementedException();
    }

    @Override
    protected String getUriParameterStringForOperation(Operation operation, CallDigits userDigits,
            Map<String, String> additionalParams) {
        throw new NotImplementedException();
    }

    private static class NotImplementedException extends UnsupportedOperationException {
        public NotImplementedException() {
            super("Only autoattendant supported for freeswitch");
        }
    }
}
