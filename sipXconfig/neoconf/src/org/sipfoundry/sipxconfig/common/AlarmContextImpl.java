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
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;

public class AlarmContextImpl implements AlarmContext {

    private String m_host;
    private AlarmApi m_alarmApi;

    @Required
    public void setHost(String host) {
        m_host = host;
    }

    @Required
    public void setAlarmApi(AlarmApi alarmApi) {
        m_alarmApi = alarmApi;
    }


    public void raiseAlarm(String alarmId, String... alarmParams) {
        try {
            m_alarmApi.raiseAlarm(m_host, alarmId, alarmParams);
        } catch (XmlRpcRemoteException e) {
            throw new UserException(e.getCause());
        }
    }

    public void reloadAlarms() {
        try {
            m_alarmApi.reloadAlarms(m_host);
        } catch (XmlRpcRemoteException e) {
            throw new UserException(e.getCause());
        }
    }
}
