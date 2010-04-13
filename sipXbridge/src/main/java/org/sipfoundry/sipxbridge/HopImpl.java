/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import javax.sip.address.Hop;

/**
 * A hop to the ITSP account. This is used by JAIN-SIP to determine where to
 * send the request.
 *
 * @author M. Ranganathan
 *
 */
public class HopImpl implements Hop {

    private String host;
    private int port;
    private String transport;
    private ItspAccountInfo itspAccountInfo;

    public HopImpl(String host, int port, String transport) {
        this.host = host;
        this.port = port;
        this.transport = transport;
    }

    public HopImpl( String host, int port, String transport, ItspAccountInfo itspAccountInfo) {
        this(host,port,transport);
        this.itspAccountInfo = itspAccountInfo;
    }

    public String getHost() {
        return host;
    }

    public int getPort() {
        return port;
    }

    public String getTransport() {
        return transport;
    }

    @Override
    public String toString() {
        return host + ":" + port + "/" + transport;
    }

    public ItspAccountInfo getItspAccountInfo() {
        return this.itspAccountInfo;
    }

}
