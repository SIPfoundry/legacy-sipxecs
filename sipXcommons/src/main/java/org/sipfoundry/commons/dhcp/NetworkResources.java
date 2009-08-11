/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dhcp;

import java.net.InetAddress;
import java.util.LinkedList;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class NetworkResources {
    public InetAddress siadr;
    public String sname;
    public String file;
    public String domainName;
    public String sipDomainName;
    public InetAddress subnetMask;
    public LinkedList<InetAddress> routers;
    public String configServer;
    public LinkedList<InetAddress> domainNameServers;
    public LinkedList<InetAddress> ntpServers;
    public int timeOffset;
    public LinkedList<SIPServer> sipServers;

}
