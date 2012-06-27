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


public class DnsTestContextImpl implements DnsTestContext {

//
//    ExternalCommand prepareCommand(boolean provideDns) {
//        ExternalCommand command = new ExternalCommand();
//        command.setContext(m_commandContext);
//        command.setCommand(m_commandString);
//        command.addArgument("-t");
//        command.addArgument(m_coreContext.getDomainName());
//        // sort locations into 3 groups
//        // - all locations with the proxyService installed
//        // - other servers in the clusters
//        // - XMPP service locations
//        List<Location> proxies = m_featureManager.getLocationsForEnabledFeature(ProxyManager.FEATURE);
//        addArgument(command, null, proxies);
//
//        List<Location> xmpps = m_featureManager.getLocationsForEnabledFeature(ImManager.FEATURE);
//        if (xmpps != null) {
//            addArgument(command, "-x", xmpps);
//        }
//        if (provideDns) {
//            command.addArgument("-p");
//        }
//        // port for XMPP?
//        //Collection<Address> xmpp = m_addressManager.getAddresses(ImManager.XMPP_ADDRESS);
//
//        // set TCP and UDP ports
//        Collection<Address> proxyTcp = m_addressManager.getAddresses(ProxyManager.TCP_ADDRESS);
//        Collection<Address> proxyUdp = m_addressManager.getAddresses(ProxyManager.UDP_ADDRESS);
//        Collection<Address> proxyTls = m_addressManager.getAddresses(ProxyManager.TLS_ADDRESS);
//        int tcpPort = proxyTcp.iterator().next().getPort();
//        int udpPort = proxyUdp.iterator().next().getPort();
//        int tlsPort = proxyTls.iterator().next().getPort();
//        command.addArgument("--port-TCP");
//        command.addArgument(String.valueOf(tcpPort));
//        command.addArgument("--port-UDP");
//        command.addArgument(String.valueOf(udpPort));
//        command.addArgument("--port-TLS");
//        command.addArgument(String.valueOf(tlsPort));
//
//        return command;
//    }
//
//    private void addArgument(ExternalCommand command, String param, Collection<Location> addresses) {
//        for (Location address : addresses) {
//            if (param != null) {
//                command.addArgument(param);
//            }
//            command.addArgument(address.getFqdn() + '/' + address.getAddress());
//        }
//    }
//
//    public void execute(boolean provideDns) {
//        ExternalCommand command = prepareCommand(provideDns);
//        // Validate current configuration
//        int returnCode = command.execute();
//        m_valid = returnCode == 0;
//        // Get error message
//        m_result = command.getStdout();
//        // Get DNS records in BIND format for sipXecs services if configuration is valid
//        if (m_valid) {
//            command.removeArgument(0);
//            command.execute();
//            m_result = command.getStdout();
//        }
//        m_runTestNeeded = false;
//    }

    @Override
    public String execute(boolean provideDns) {
        return "TBD";
    }
}
