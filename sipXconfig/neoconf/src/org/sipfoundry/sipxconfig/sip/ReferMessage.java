/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.sip;

import java.io.IOException;
import java.util.Formatter;
import java.util.Locale;

class ReferMessage extends Message {

    private String m_sourceAddrSpec;
    private String m_destinationAddSpec;

    public ReferMessage(String serverName, String sourceAddrSpec, String destinationAddrSpec) {
        super(serverName);
        m_sourceAddrSpec = sourceAddrSpec;
        m_destinationAddSpec = destinationAddrSpec;
    }

    void formatHeaders(Appendable buf, String proxyHost, int proxyPort) throws IOException {
        long uniqueId = generateUniqueId();
        Formatter f = new Formatter(buf, Locale.US);
        f.format("REFER %s SIP/2.0\r\n", m_sourceAddrSpec);
        addVia(f, proxyHost, proxyPort, uniqueId);
        addTo(f, m_sourceAddrSpec);
        addFrom(f);
        f.format("Call-ID: 90d3f3-%x\r\n", uniqueId);
        buf.append("CSeq: 1 REFER\r\n");
        f.format("Refer-To: %s\r\n", m_destinationAddSpec);
        addCommonHeaders(f, proxyHost, proxyPort);
    }
}
