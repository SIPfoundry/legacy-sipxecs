/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.util.Properties;

import javax.sip.message.Response;

import org.apache.log4j.PropertyConfigurator;
import org.sipfoundry.sipxbridge.Gateway;
import org.sipfoundry.sipxbridge.ItspAccountInfo;
import org.cafesip.sipunit.SipCall;
import org.cafesip.sipunit.SipPhone;
import org.cafesip.sipunit.SipResponse;
import org.cafesip.sipunit.SipStack;
import org.cafesip.sipunit.SipTestCase;
import org.cafesip.sipunit.SipTransaction;


import junit.framework.TestCase;

public class SimpleCallSetupTest extends SipTestCase {

    private MockItsp mockItsp;
    private SipStack sipStack;
    private SipPhone caller;
    private ItspAccountInfo accountInfo;
    
    private static String sdpBodyFormat = "v=0\r\n"
        + "o=- 3408914179 3408914179 IN IP4 %s\r\n"
        + "s=iBridge\r\n" + "c=IN IP4 %s\r\n" + "t=0 0\r\n"
        + "a=direction:active\r\n"
        + "m=audio 49156 RTP/AVP 3 97 98 110 8 0 101\r\n"
        + "a=rtpmap:3 GSM/8000\r\n" + "a=rtpmap:97 iLBC/8000\r\n"
        + "a=rtpmap:98 iLBC/8000\r\n" + "a=fmtp:98 mode=20\r\n"
        + "a=rtpmap:110 speex/8000\r\n" + "a=rtpmap:8 PCMA/8000\r\n"
        + "a=rtpmap:0 PCMU/8000\r\n"
        + "a=rtpmap:101 telephone-event/8000\r\n"
        + "a=fmtp:101 0-11,16\r\n";

    public void testSendInviteFromSipxProxy() {
        try {
            String to="sip:3015551212@" + accountInfo.getProxyDomain();
            String sdpBody = String.format(sdpBodyFormat, 
                    Gateway.getLocalAddress(), Gateway.getLocalAddress());
            System.out.println("sdp = " + sdpBody);
            SipCall sipCall = caller.makeCall(to, Response.OK, 
                    1000,null,sdpBody,"application","sdp",null,null);
            assertNotNull("Null sipCall ", sipCall);
            super.assertLastOperationSuccess("Expect an OK" + sipCall.format(), sipCall);
          
            
        } catch (Exception ex) {
            ex.printStackTrace();
            fail("Unexpected exception occured");

        }

    }

    @Override
    public void tearDown() throws Exception {
        mockItsp.stop();

        Gateway.stop();
        
    }

    @Override
    public void setUp() throws Exception {
        super.setUp();
        Properties properties = new Properties();
        properties.load(new FileInputStream ( new File ( "testdata/selftest.properties")));
        String accountName = properties.getProperty("org.sipfoundry.gateway.mockItspAccount");
        Gateway.setConfigurationFileName(accountName);
        System.out.println("config file name " + accountName);
        Gateway.startXmlRpcServer();
        accountInfo  = 
                Gateway.getAccountManager().getDefaultAccount();
        
        
        int sipxProxyPort = Integer.parseInt(properties.getProperty("org.sipfoundry.gateway.mockSipxProxyPort"));
        sipStack = new SipStack("udp",sipxProxyPort);
        
        String localAddr =  Gateway.getAccountManager().getBridgeConfiguration().getLocalAddress();
        int localPort = Gateway.getAccountManager().getBridgeConfiguration().getLocalPort();
        
        System.out.println("localAddr = " + localAddr);
        System.out.println("localPort = " + localPort);
        caller = sipStack.createSipPhone(localAddr, "udp", localPort, "sip:mranga@pingtel.com");
        this.mockItsp = new MockItsp();
        this.mockItsp.init(1000);
        Gateway.start();
        
        

    }

}
