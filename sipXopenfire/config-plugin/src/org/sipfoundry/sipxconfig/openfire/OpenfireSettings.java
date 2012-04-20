/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.openfire;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class OpenfireSettings extends PersistableSettings implements DeployConfigOnEdit {
    private static final String LOCALE = "settings/locale";
    private static final String LOG_LEVEL = "settings/log.level";
    private static final String WATCHER_PORT = "settings/watcher-port";
    private static final String XML_RPC_PORT = "settings/openfire-xml-rpc-port";
    private static final String XML_RPC_VCARD_PORT = "settings/openfire-xml-rpc-vcard-port";
    private static final String SERVER_TO_SERVER_ENABLED = "openfire-server-to-server/enabled";
    private static final String ALLOWED_SERVERS = "openfire-server-to-server/allowed-servers";
    private static final String DISCONNECT_ON_IDLE = "openfire-server-to-server/disconnect-on-idle";
    private static final String IDLE_TIMEOUT = "openfire-server-to-server/idle-timeout";
    private static final String ANY_CAN_CONNECT = "openfire-server-to-server/any-can-connect";
    private static final String DISALLOWED_SERVERS =
        "openfire-server-to-server/disallowed-servers";
    private static final int XMPP_PORT = 5269; // not configurable
    private static final String MESSAGE_LOG_ENABLED = "message-logging/enabled";
    private static final AddressType GENERIC_ADDRESS = new AddressType("generic");
    private LocalizationContext m_localizationContext;
    private String m_logDir;

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("openfire/openfire.xml");
    }

    public int getWatcherPort() {
        return (Integer) getSettingTypedValue(WATCHER_PORT);
    }

    public int getXmlRpcPort() {
        return (Integer) getSettingTypedValue(XML_RPC_PORT);
    }
    
    public int getXmlRpcVcardPort() {
        return (Integer) getSettingTypedValue(XML_RPC_VCARD_PORT);
    }    

    public int getXmppPort() {
        return XMPP_PORT;
    }

    public List<Address> getAllowedServers() {
        String value = getSettingValue(ALLOWED_SERVERS);
        return parseServerArray(value);
    }

    public List<Address> getDisallowedServers() {
        String value = getSettingValue(DISALLOWED_SERVERS);
        // not sure why disallowed includes ports, but keeping it as is was when i found it --Douglas
        return parseServerArray(value);
    }

    public String getLocale() {
        return (String) getSettingTypedValue(LOCALE);
    }

    public String getLogLevel() {
        return (String) getSettingTypedValue(LOG_LEVEL);
    }

    public String getServerToServer() {
        return (String) getSettingTypedValue(SERVER_TO_SERVER_ENABLED).toString();
    }

    public String getDisconnectOnIdle() {
        return (String) getSettingTypedValue(DISCONNECT_ON_IDLE).toString();
    }

    public Integer getIdleTimeout() {
        return (Integer) getSettingTypedValue(IDLE_TIMEOUT);
    }

    public String getAnyCanConnect() {
        return (String) getSettingTypedValue(ANY_CAN_CONNECT).toString();
    }

    public String getMessageLogEnabled() {
        return (String) getSettingTypedValue(MESSAGE_LOG_ENABLED).toString();
    }

    public String getLogDir() {
        return m_logDir;
    }

    /**
     * Example:
     * Given
     *   foo,bar:1235,goose
     * Return
     *   { Address("foo", 5096), Address("bar", 1235), Address("goose", 5096) }
     */
    List<Address> parseServerArray(String value) {
        if (StringUtils.isBlank(value)){
            return Collections.emptyList();
        }
        List<Address> servers = new ArrayList<Address>();
        String[] strServers = StringUtils.split(value, ',');
        for (String strServer : strServers) {
            String[] hostPort = StringUtils.split(strServer, ':');
            Address address = new Address(GENERIC_ADDRESS, hostPort[0]);
            if (hostPort.length < 2) {
                address.setPort(getXmppPort());
            } else {
                try {
                    address.setPort(Integer.parseInt(hostPort[1]));
                } catch (NumberFormatException e) {
                    address.setPort(getXmppPort());
                }
            }
            servers.add(address);
        }

        return servers;
    }

    class Defaults {
        @SettingEntry(path = LOCALE)
        public String getLocale() {
            return m_localizationContext.getLocalization().getLanguage();
        }
    }

    public void setLocalizationContext(LocalizationContext localizationContext) {
        m_localizationContext = localizationContext;
    }

    public void setLogDir(String dir) {
        m_logDir = dir;
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) OpenfireImpl.FEATURE);
    }

    @Override
    public String getBeanId() {
        return "openfireSettings";
    }
}
