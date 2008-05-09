/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;


import java.io.File;
import java.io.FileInputStream;
import java.util.Properties;

import javax.sip.ListeningPoint;
import javax.sip.SipListener;
import javax.sip.SipProvider;

import org.cafesip.sipunit.SipTestCase;
import org.sipfoundry.sipxbridge.*;

import gov.nist.javax.sip.clientauthutils.*;

import junit.framework.TestCase;

public abstract class AbstractSipSignalingTest extends SipTestCase {
    protected int sipxProxyPort;
    protected String localAddr;
    protected int localPort;
    private static int baseMediaPort;
    protected ItspAccountInfo accountInfo;
    protected AccountManagerImpl accountManager;
    
    @Override
    public void setUp() throws Exception {
        Properties properties = new Properties();
        properties.load(new FileInputStream(new File(
                "testdata/selftest.properties")));
        String accountName = properties
                .getProperty("org.sipfoundry.gateway.mockItspAccount");
        Gateway.setConfigurationFileName(accountName);
        System.out.println("config file name " + accountName);
        Gateway.startXmlRpcServer();
        accountManager = Gateway.getAccountManager();
        accountInfo = accountManager.getDefaultAccount();

        sipxProxyPort = Integer.parseInt(properties
                .getProperty("org.sipfoundry.gateway.mockSipxProxyPort"));
       
        localAddr = Gateway.getAccountManager().getBridgeConfiguration()
                .getLocalAddress();
        localPort = Gateway.getAccountManager().getBridgeConfiguration()
                .getLocalPort();
        baseMediaPort = Integer.parseInt
        (properties.getProperty("org.sipfoundry.gateway.mediaBasePort"));
    }

    @Override
    public void tearDown() throws Exception {
        Gateway.stop();

    }
    
    public int getMediaPort() {
        return baseMediaPort++;
    }

}
