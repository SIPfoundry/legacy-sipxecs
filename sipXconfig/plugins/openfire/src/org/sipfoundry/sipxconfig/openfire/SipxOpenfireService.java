/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.openfire;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.service.LoggingEntity;
import org.sipfoundry.sipxconfig.service.SipxRegistrarConfiguration;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

import static org.springframework.dao.support.DataAccessUtils.singleResult;

public class SipxOpenfireService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxOpenfireService";
    public static final String LOG_SETTING = "settings/log-level";
    public static final String HOST_SETTING = "settings/openfire-host";
    public static final String WATCHER_SETTING = "settings/watcher-address";
    public static final String DOMAIN_SETTING = "settings/sipx-proxy-domain";
    public static final String LOCALE_SETTING = "settings/locale";
    public static final String XML_RPC_PORT_SETTING = "settings/openfire-xml-rpc-port";
    public static final String SERVER_TO_SERVER_ALLOWED_SERVERS_SETTING = "openfire-server-to-server/allowed-servers";
    public static final String SERVER_TO_SERVER_DISALLOWED_SERVERS_SETTING =
        "openfire-server-to-server/disallowed-servers";
    public static final String SERVER_TO_SERVER_DEFAULT_REMOTE_PORT =
        "openfire-server-to-server/default-remote-server-port";

    public static final String DEBUG = "DEBUG";
    public static final String INFO = "INFO";
    public static final String NOTICE = "NOTICE";
    public static final String WARNING = "WARNING";
    public static final String ERR = "ERR";
    public static final String CRIT = "CRIT";
    public static final String ALERT = "ALERT";
    public static final String EMERG = "EMERG";

    private LocationsManager m_locationsManager;

    @Override
    public String getLogSetting() {
        return LOG_SETTING;
    }

    @Override
    public void initialize() {
        List<Location> locations = m_locationsManager.getLocationsForService(this);
        addDefaultBeanSettingHandler(new Defaults((Location) singleResult(locations), getDomainName()));
    }

    @Override
    public void setLogLevel(String logLevel) {
        super.setLogLevel(logLevel);
    }

    @Override
    public String getLogLevel() {
        return super.getLogLevel();
    }

    @Override
    public String getLabelKey() {
        return super.getLabelKey();
    }

    public String getServerAddress() {
        return getSettings().getSetting(HOST_SETTING).getValue();
    }

    public int getPort() {
        return (Integer) getSettings().getSetting(XML_RPC_PORT_SETTING).getTypedValue();
    }

    public void setLocale(String localeString) {
        getSettings().getSetting(LOCALE_SETTING).setValue(localeString);
    }

    public static class Defaults {
        private final Location m_location;
        private final String m_domainName;

        Defaults(Location location, String domainName) {
            m_location = location;
            m_domainName = domainName;
        }

        @SettingEntry(path = WATCHER_SETTING)
        public String getHostAddress() {
            return m_location.getAddress();
        }

        @SettingEntry(path = HOST_SETTING)
        public String getHostFqdn() {
            return m_location.getFqdn();
        }

        @SettingEntry(path = DOMAIN_SETTING)
        public String getDomainAddress() {
            return m_domainName;
        }
    }

    @Override
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Override
    public Object getParam(String paramName) {
        if (paramName.equals(SipxRegistrarConfiguration.OPENFIRE_HOST)) {
            return getServerAddress();
        } else if (paramName.equals(SipxRegistrarConfiguration.OPENFIRE_XML_RPCPORT)) {
            return getPort();
        }
        return super.getParam(paramName);
    }

    public static Map<String, String> getLogLevelsMappings() {
        Map<String, String> mappings = new HashMap<String, String>();
        mappings.put(DEBUG, DEBUG);
        mappings.put(INFO, INFO);
        mappings.put(NOTICE, INFO);
        mappings.put(WARNING, WARNING);
        mappings.put(ERR, ERR);
        mappings.put(CRIT, ERR);
        mappings.put(ALERT, ERR);
        mappings.put(EMERG, ERR);
        return mappings;
    }
}
