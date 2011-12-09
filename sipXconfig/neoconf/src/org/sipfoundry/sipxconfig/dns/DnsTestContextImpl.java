/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.dns;

import java.util.ArrayList;
import java.util.Collection;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.configdiag.ExternalCommand;
import org.sipfoundry.sipxconfig.configdiag.ExternalCommandContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.springframework.beans.factory.annotation.Required;

public class DnsTestContextImpl implements DnsTestContext, DaoEventListener {
    private String m_result;
    private boolean m_valid;
    private boolean m_runTestNeeded = true;
    private CoreContext m_coreContext;
    private AddressManager m_addressManager;

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
        Collection<Address> addresses = new ArrayList<Address>();
        Collection<Address> proxyTcp = m_addressManager.getAddresses(ProxyManager.TCP_ADDRESS);
        Collection<Address> proxyUdp = m_addressManager.getAddresses(ProxyManager.UDP_ADDRESS);
        Collection<Address> proxyTls = m_addressManager.getAddresses(ProxyManager.TLS_ADDRESS);
        int tcpPort = proxyTcp.iterator().next().getPort();
        int udpPort = proxyUdp.iterator().next().getPort();
        int tlsPort = proxyTls.iterator().next().getPort();
        addArgument(command, null, proxyTcp);
        addArgument(command, null, proxyUdp);
        addArgument(command, null, proxyTls);
        Collection<Address> xmpp = m_addressManager.getAddresses(ImManager.XMPP_ADDRESS);
        addArgument(command, "-x", xmpp);
        if (provideDns) {
            command.addArgument("-p");
        }

        // set TCP and UDP ports
        command.addArgument("--port-TCP");
        command.addArgument(String.valueOf(tcpPort));
        command.addArgument("--port-UDP");
        command.addArgument(String.valueOf(udpPort));
        command.addArgument("--port-TLS");
        command.addArgument(String.valueOf(tlsPort));

        return command;
    }

    private void addArgument(ExternalCommand command, String param, Collection<Address> addresses) {
        for (Address address : addresses) {
            if (param != null) {
                command.addArgument(param);
            }
            command.addArgument(address.getAddress() + '\'' + address.getPort());
        }
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

    @Required
    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }
}
