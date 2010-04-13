/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxbridge;

import java.io.File;
import java.io.FileInputStream;
import java.util.Properties;

import junit.framework.TestCase;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxbridge.xmlrpc.RegistrationRecord;
import org.sipfoundry.sipxbridge.xmlrpc.SipXbridgeXmlRpcClient;

public class InvalidAccountTest extends TestCase {

    private static Logger logger = Logger.getLogger(InvalidAccountTest.class);
    private static String serverAddress;
    private static  int port ;
    private SipXbridgeXmlRpcClient client;

    /*
     * (non-Javadoc)
     * 
     * @see junit.framework.TestCase#setUp()
     */

    protected void setUp() throws Exception {
       
        super.setUp();
        Properties properties = new Properties();
        properties.load(new FileInputStream( new File ("testdata/selftest.properties")));
        Gateway.setConfigurationFileName(properties.getProperty("org.sipfoundry.gateway.badAccount"));
       
        Gateway.parseConfigurationFile();
        SipXbridgeXmlRpcServerImpl.startXmlRpcServer();
        System.out.println("Web server started");
        client = new SipXbridgeXmlRpcClient
            (Gateway.getAccountManager().getBridgeConfiguration().getExternalAddress(),
                    Gateway.getAccountManager().getBridgeConfiguration().getXmlRpcPort(),
                    Gateway.getBridgeConfiguration().isSecure());
        
        
       

    }

    public void testBadAccountInfo() throws Exception {
        try {
            client.start();
            System.out.println("SipXbridge Started");
            Thread.sleep(2000);
            RegistrationRecord[] registrationRecords = client.getRegistrationRecords();
            System.out.println("accountState is " + registrationRecords[0].getRegistrationStatus());
            assertEquals(registrationRecords.length, 1 );
            assertTrue ("Should not be authenticated ", registrationRecords[0].getRegistrationStatus().equals(AccountState.AUTHENTICATION_FAILED.toString()) ||
                    registrationRecords[0].getRegistrationStatus().equals(AccountState.AUTHENTICATING.toString()) );
            
        } catch ( Exception ex) {
            ex.printStackTrace();
            throw ex;
        }
        
    }

    protected void tearDown() throws Exception {
        super.tearDown();
        client.stop();
        SipXbridgeXmlRpcServerImpl.stopXmlRpcServer();
    }

}
