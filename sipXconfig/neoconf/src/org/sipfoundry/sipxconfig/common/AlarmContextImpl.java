/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import org.sipfoundry.sipxconfig.admin.commserver.AlarmApi;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;

public class AlarmContextImpl implements AlarmContext {

    private String m_host;
    private String m_alarmServerUrl;
    private ApiProvider<AlarmApi> m_alarmApiProvider;

    @Required
    public void setHost(String host) {
        m_host = host;
    }

    @Required
    public void setAlarmServerUrl(String alarmServerUrl) {
        m_alarmServerUrl = alarmServerUrl;
    }

    @Required
    public void setAlarmApiProvider(ApiProvider<AlarmApi> alarmApiProvider) {
        m_alarmApiProvider = alarmApiProvider;
    }


    public void raiseAlarm(String alarmId, String... alarmParams) {
        try {
            AlarmApi api = m_alarmApiProvider.getApi(m_alarmServerUrl);
            api.raiseAlarm(m_host, alarmId, alarmParams);
        } catch (XmlRpcRemoteException e) {
            throw new UserException(e.getCause());
        }
    }

    public void reloadAlarms() {
        try {
            AlarmApi api = m_alarmApiProvider.getApi(m_alarmServerUrl);
            api.reloadAlarms(m_host);
        } catch (XmlRpcRemoteException e) {
            throw new UserException(e.getCause());
        }
    }
}
