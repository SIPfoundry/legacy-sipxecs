/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.openfire;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.service.SipxRestService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceConfiguration;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.springframework.beans.factory.annotation.Required;

import static org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType.XMPP_SERVER;

public class SipxOpenfireConfiguration extends SipxServiceConfiguration {
    private static final String HOST_PORT_SEPERATOR = ":";
    private static final int SERVER_TO_SERVER_PORT_MIN = 1024;
    private static final int SERVER_TO_SERVER_PORT_MAX = 65535;

    private CoreContext m_coreContext;

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        SipxService openfireService = getSipxOpenfireService();
        context.put("settings", openfireService.getSettings());
        context.put("service", openfireService);

        SipxService restService = getService(SipxRestService.BEAN_ID);
        context.put("restService", restService);

        String username = XMPP_SERVER.getUserName();
        User user = m_coreContext.getSpecialUser(XMPP_SERVER);
        context.put("username", username);
        context.put("password", user.getSipPassword());
        context.put("resource-list", SpeedDial.getResourceListId(username, true));

        List<Map<String, String>> allowedServers = getServers(openfireService.getSettings().getSetting(
                SipxOpenfireService.SERVER_TO_SERVER_ALLOWED_SERVERS_SETTING).getValue());
        List<Map<String, String>> disallowedServers = getServers(openfireService.getSettings().getSetting(
                SipxOpenfireService.SERVER_TO_SERVER_DISALLOWED_SERVERS_SETTING).getValue());
        context.put("allowedServers", allowedServers);
        context.put("disallowedServers", disallowedServers);

        return context;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    private List<Map<String, String>> getServers(String settingValue) {
        List<Map<String, String>> serversList = new ArrayList<Map<String, String>>();
        if (settingValue == null) {
            return serversList;
        }

        Set<String> servers = new LinkedHashSet<String>();
        String[] rawServersArray = StringUtils.split(settingValue, ",");

        for (String rawServer : rawServersArray) {
            String server = rawServer.trim();
            String port = StringUtils.substringAfterLast(server, HOST_PORT_SEPERATOR);
            if (port.isEmpty() || Integer.valueOf(port) < SERVER_TO_SERVER_PORT_MIN
                    || Integer.valueOf(port) > SERVER_TO_SERVER_PORT_MAX) {
                port = String.valueOf(getSipxOpenfireService().getSettingTypedValue(
                        SipxOpenfireService.SERVER_TO_SERVER_DEFAULT_REMOTE_PORT));
            }

            servers.add(StringUtils.substringBeforeLast(server, HOST_PORT_SEPERATOR) + HOST_PORT_SEPERATOR
                    + port);
        }

        for (String server : servers) {
            Map<String, String> map = new HashMap<String, String>();
            map.put("host", StringUtils.substringBeforeLast(server, HOST_PORT_SEPERATOR));
            map.put("port", StringUtils.substringAfterLast(server, HOST_PORT_SEPERATOR));
            serversList.add(map);
        }

        return serversList;
    }

    @Override
    public boolean isReplicable(Location location) {
        return getSipxServiceManager().isServiceInstalled(location.getId(), SipxOpenfireService.BEAN_ID);
    }

    private SipxService getSipxOpenfireService() {
        return getService(SipxOpenfireService.BEAN_ID);
    }
}
