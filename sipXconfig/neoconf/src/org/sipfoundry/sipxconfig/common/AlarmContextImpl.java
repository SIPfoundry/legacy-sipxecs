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
import org.sipfoundry.sipxconfig.admin.commserver.AlarmApi;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;

public class AlarmContextImpl implements AlarmContext {
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
            Location primaryLocation = getPrimaryLocation();
            getAlarmApi(primaryLocation).raiseAlarm(primaryLocation.getFqdn(), alarmId,
                    ParamsUtils.escape(alarmParams));
        } catch (XmlRpcRemoteException e) {
            throw new UserException(e.getCause());
        }
    }

    public void reloadAlarms() {
        try {
            Location[] locations = m_locationsManager.getLocations();
            for (Location location : locations) {
                getAlarmApi(location).reloadAlarms(getPrimaryLocation().getFqdn());
            }
        } catch (XmlRpcRemoteException e) {
            throw new UserException(e.getCause());
        }
    }

    private Location getPrimaryLocation() {
        return m_locationsManager.getPrimaryLocation();
    }

    private AlarmApi getAlarmApi(Location location) {
        return m_alarmApiProvider.getApi(location.getProcessMonitorUrl());
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
