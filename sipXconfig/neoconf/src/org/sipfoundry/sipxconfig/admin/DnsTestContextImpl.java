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
    private static final String SLASH = "/";
    private String m_result;
    private LocationsManager m_locationsManager;
    private SipxServiceManager m_sipxServiceManager;
    private final ExternalCommand m_command;
    private boolean m_valid;
    private CoreContext m_coreContext;
    private boolean m_runTestNeeded = true;

    public DnsTestContextImpl(ExternalCommandContext commandContext, String commandString) {
        m_command = new ExternalCommand();
        m_command.setContext(commandContext);
        m_command.setCommand(commandString);
    }

    public String getResult() {
        return m_result;
    }

    public void execute(boolean provideDns) {
        String sipDomain = m_coreContext.getDomainName();
        Location[] locations = m_locationsManager.getLocations();
        SipxService proxyService = m_sipxServiceManager.getServiceByBeanId(SipxProxyService.BEAN_ID);
        m_command.clearArguments();
        m_command.addArgument("-t");
        m_command.addArgument(sipDomain);
        for (Location location : locations) {
            if (location.isServiceInstalled(proxyService)) {
                //call servers should be added first
                m_command.addArgument(location.getFqdn() + SLASH + location.getAddress(), 2);
            } else {
                //other servers should be added last
                m_command.addArgument("-o");
                m_command.addArgument(location.getFqdn() + SLASH + location.getAddress());
            }
        }
        if (provideDns) {
            m_command.addArgument("-p");
        }
        //Validate current configuration
        int returnCode = m_command.execute();
        m_valid = returnCode == 0 ? true : false;
        //Get error message
        m_result = m_command.getStdout();
        //Get DNS records in BIND format for sipXecs services if configuration is valid
        if (m_valid) {
            m_command.removeArgument(0);
            m_command.execute();
            m_result = m_command.getStdout();
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

    public ExternalCommand getCommand() {
        return m_command;
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
