/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.common;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServerActivatedEvent;
import org.sipfoundry.sipxconfig.admin.commserver.AlarmApi;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class AlarmContextImpl implements AlarmContext, ApplicationListener {
    private ApiProvider<AlarmApi> m_alarmApiProvider;
    private LocationsManager m_locationsManager;

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setAlarmApiProvider(ApiProvider<AlarmApi> alarmApiProvider) {
        m_alarmApiProvider = alarmApiProvider;
    }

    public void raiseAlarm(String alarmId, String... alarmParams) {
        try {
            getAlarmApi().raiseAlarm(getHost(), alarmId, ParamsUtils.escape(alarmParams));
        } catch (XmlRpcRemoteException e) {
            throw new UserException(e.getCause());
        }
    }

    public void reloadAlarms() {
        try {
            getAlarmApi().reloadAlarms(getHost());
        } catch (XmlRpcRemoteException e) {
            throw new UserException(e.getCause());
        }
    }

    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof AlarmServerActivatedEvent) {
            // send "reloadAlarms" command to the supervisor
            reloadAlarms();
        }
    }

    private String getHost() {
        return m_locationsManager.getPrimaryLocation().getFqdn();
    }

    private AlarmApi getAlarmApi() {
        return m_alarmApiProvider.getApi(m_locationsManager.getPrimaryLocation().getProcessMonitorUrl());
    }

    static final class ParamsUtils {
        private static final String[] SEARCH = {
            "\n", "\t"
        };
        private static final String[] REPLACEMENTS = {
            "&#xA;", "&#x9;"
        };

        private ParamsUtils() {
            // utility class
        }

        /**
         * Encode whitespace in alarm parameters so that is not removed by XML parser.
         */
        static String[] escape(String... alarmParams) {
            String[] escapedParams = new String[alarmParams.length];
            for (int i = 0; i < escapedParams.length; i++) {
                escapedParams[i] = StringUtils.replaceEach(alarmParams[i], SEARCH, REPLACEMENTS);
            }
            return escapedParams;
        }
    }
}
