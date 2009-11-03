/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.configdiag.ExternalCommand;
import org.sipfoundry.sipxconfig.admin.configdiag.ExternalCommandContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;

public class DnsTestContextImpl implements DnsTestContext, DaoEventListener {
    private String m_result;
    private boolean m_valid;
    private boolean m_runTestNeeded = true;

    private LocationsManager m_locationsManager;
    private SipxServiceManager m_sipxServiceManager;
    private CoreContext m_coreContext;

    private final ExternalCommandContext m_commandContext;
    private final String m_commandString;

    public DnsTestContextImpl(ExternalCommandContext commandContext, String commandString) {
        m_commandContext = commandContext;
        m_commandString = commandString;
    }

    public String getResult() {
        return m_result;
    }

    ExternalCommand prepareCommand(boolean provideDns) {
        ExternalCommand command = new ExternalCommand();
        command.setContext(m_commandContext);
        command.setCommand(m_commandString);
        command.addArgument("-t");
        command.addArgument(m_coreContext.getDomainName());
        // sort locations into 3 groups
        // - all locations with the proxyService installed
        // - other servers in the clusters
        // - XMPP service locations
        Location[] locations = m_locationsManager.getLocations();
        SipxService proxyService = m_sipxServiceManager.getServiceByBeanId(SipxProxyService.BEAN_ID);
        SipxService openFireService = m_sipxServiceManager.getServiceByBeanId("sipxOpenfireService");
        List<String> proxyArgs = new ArrayList<String>(locations.length);
        List<String> otherArgs = new ArrayList<String>(locations.length);
        List<String> xmppArgs = new ArrayList<String>(locations.length);
        for (Location location : locations) {
            String arg = String.format("%s/%s", location.getFqdn(), location.getAddress());
            if (location.isServiceInstalled(proxyService)) {
                proxyArgs.add(arg);
            } else {
                otherArgs.add(arg);
            }
            if (location.isServiceInstalled(openFireService)) {
                xmppArgs.add(arg);
            }
        }
        for (String arg : proxyArgs) {
            command.addArgument(arg);
        }
        for (String arg : otherArgs) {
            command.addArgument("-o");
            command.addArgument(arg);
        }
        for (String arg : xmppArgs) {
            command.addArgument("-x");
            command.addArgument(arg);
        }
        if (provideDns) {
            command.addArgument("-p");
        }

        // set TCP and UDP ports
        command.addArgument("--port-TCP");
        command.addArgument(((SipxProxyService) proxyService).getSipTCPPort());
        command.addArgument("--port-UDP");
        command.addArgument(((SipxProxyService) proxyService).getSipUDPPort());
        return command;
    }

    public void execute(boolean provideDns) {
        ExternalCommand command = prepareCommand(provideDns);
        // Validate current configuration
        int returnCode = command.execute();
        m_valid = returnCode == 0;
        // Get error message
        m_result = command.getStdout();
        // Get DNS records in BIND format for sipXecs services if configuration is valid
        if (m_valid) {
            command.removeArgument(0);
            command.execute();
            m_result = command.getStdout();
        }
        m_runTestNeeded = false;
    }

    public boolean isValid() {
        return m_valid;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void onDelete(Object entity) {
        if (entity instanceof Location) {
            m_runTestNeeded = true;
        }
    }

    public void onSave(Object entity) {
        if (entity instanceof Location) {
            m_runTestNeeded = true;
        }
    }

    public boolean isRunTestNeeded() {
        return m_runTestNeeded;
    }
}
