/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.openfire;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class OpenfireSettings extends BeanWithSettings {
    private static final String LOCALE = "settings/locale";
    private static final String WATCHER_PORT = "settings/watcher-port";
    private static final String XML_RPC_PORT = "settings/openfire-xml-rpc-port";
    private static final String ALLOWED_SERVERS = "openfire-server-to-server/allowed-servers";
    private static final String DISALLOWED_SERVERS =
        "openfire-server-to-server/disallowed-servers";
    private static final int XMPP_PORT = 5269; // not configurable 
    private LocalizationContext m_localizationContext;

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
            Address address = new Address(hostPort[0]);
            if (hostPort.length < 2) {
                address.setPort(getXmppPort());
            } else {
                try {
                    address.setPort(Integer.parseInt(hostPort[1]));
                } catch (NumberFormatException e) {
                    address.setPort(getXmppPort());
                }
            }
        }
        
        return servers;
    }
    
    class Defaults {
        @SettingEntry(path = LOCALE)
        public String getLocale() {
            return m_localizationContext.getLocalization().getLanguage();
        }
    }
}
