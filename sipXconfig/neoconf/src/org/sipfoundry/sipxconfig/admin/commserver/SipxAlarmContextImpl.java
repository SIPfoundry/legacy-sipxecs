/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Vector;

//import org.apache.commons.logging.Log;
//import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;

public class SipxAlarmContextImpl implements SipxAlarmContext {

    private String m_host;
    private LocationsManager m_locationsManager;
    private ApiProvider<AlarmApi> m_alarmApiProvider;

    @Required
    public void setHost(String host) {
        m_host = host;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setAlarmApiProvider(ApiProvider<AlarmApi> alarmApiProvider) {
        m_alarmApiProvider = alarmApiProvider;
    }

    
    public void raiseAlarm(String alarmId, Vector<String> alarmParams) {
        // sending the alarm to the local sipxsupervisor, not the HA master
        Location[] locations = m_locationsManager.getLocations();
        Location location = locations[0];
        try {
            // Break the result into the keys and values.
            AlarmApi api = m_alarmApiProvider.getApi(location.getProcessMonitorUrl());
            api.raiseAlarm(m_host, alarmId, alarmParams);
        } catch (XmlRpcRemoteException e) {
            throw new UserException(e.getCause());
        }
    }
    
    public void reloadAlarms() {
        // sending the request to the local sipxsupervisor, not the HA master
        Location[] locations = m_locationsManager.getLocations();
        Location location = locations[0];
        try {
            // Break the result into the keys and values.
            AlarmApi api = m_alarmApiProvider.getApi(location.getProcessMonitorUrl());
            api.reloadAlarms(m_host);
        } catch (XmlRpcRemoteException e) {
            throw new UserException(e.getCause());
        }
    }

}
