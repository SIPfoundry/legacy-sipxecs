/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.io.IOException;
import java.text.DateFormat;

/**
 * Sip utility service functions complete w/server current configuration
 */
public interface SipService {
    
    public String getServerVia();
    
    public String getServerUri();
    
    public void send(byte[] sipMsg) throws IOException;
    
    public void sendCheckSync(String uri, String outboundProxy, 
            String outboundProxyPort);

    public void sendNotify(String uri, String outboundProxy, 
         String outboundProxyPort, String event, byte[] payload);

    public String getCurrentDate();
    
    public void setProxyHost(String proxy);

    public void setProxyPort(int port);
    
    /**
     * To format your own date in SIP RFC format
     * Example:  Tue, 15 Nov 1994 08:12:31 GMT 
     */
    public DateFormat getDateFormat();
    
    public String generateCallId();
    
}
