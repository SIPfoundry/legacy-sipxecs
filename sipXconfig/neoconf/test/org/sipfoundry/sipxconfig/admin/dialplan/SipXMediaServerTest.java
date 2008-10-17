/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.dialplan.MediaServer.Operation;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public class SipXMediaServerTest extends TestCase {

    private static final String VOICEMAIL_SERVER = "https%3A%2F%2Flocalhost%3A443";
    private SipXMediaServer m_out;
    
    public void setUp() {
        m_out = new SipXMediaServer();
        SipXMediaServerTest.configureMediaServer(m_out);
    }

    public void testGetHeaderParams() {
        assertNull("Header params should be null", 
                m_out.getHeaderParameterStringForOperation(null, null, null));
    }
    
    public void testGetUriParamsForVoicemailRetrieve() {
        String paramsForRetrieve = m_out.getUriParameterStringForOperation(
                Operation.VoicemailRetrieve, null, null);
        assertEquals("Wrong uri param string.", 
                "voicexml=" + VOICEMAIL_SERVER + "%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Dretrieve",
                paramsForRetrieve);
    }
    
    public void testGetUriParamsForVoicemailDeposit() {
        String paramsForRetrieve = m_out.getUriParameterStringForOperation(
                Operation.VoicemailDeposit, CallDigits.VARIABLE_DIGITS, null);
        assertEquals("Wrong uri param string.", 
                "voicexml=" + VOICEMAIL_SERVER + "%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Ddeposit%26mailbox%3D{vdigits-escaped}",
                paramsForRetrieve);
    }
    
    public void testGetUriParamsForAutoattendant() {
        Map<String, String> additionalParams = new HashMap<String, String>();
        additionalParams.put("name", "operator");
        String paramsForRetrieve = m_out.getUriParameterStringForOperation(
                Operation.Autoattendant, null, additionalParams);
        assertEquals("Wrong uri param string.", 
                "voicexml=" + VOICEMAIL_SERVER + "%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Dautoattendant%26name%3Doperator",
                paramsForRetrieve);
    }
    
    public void testGetDigitsForOperation() {
        assertEquals("Wrong digits for deposit with {digits}.",
                "{digits}", m_out.getDigitStringForOperation(
                        Operation.VoicemailDeposit, CallDigits.FIXED_DIGITS));
        assertEquals("Wrong digits for deposit with {vdigits}.",
                "{vdigits}", m_out.getDigitStringForOperation(
                        Operation.VoicemailDeposit, CallDigits.VARIABLE_DIGITS));
    }
    
    public void testGetHostname() {
        m_out.setHostname("foo");
        assertEquals("Wrong server address.", "localhost;transport=tcp", m_out.getHostname());
    }

    /**
     * Configure an instance of SipXMediaServer with necessary dependencies for test purposes only.
     */
    public static final void configureMediaServer(SipXMediaServer mediaServer) {
        SipxRegistrarService registrarService = new SipxRegistrarService();
        registrarService.setMediaServerSipSrvOrHostport("localhost");
        registrarService.setVoicemailHttpsPort("443");
        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxRegistrarService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(registrarService).anyTimes();
        EasyMock.replay(sipxServiceManager);
        mediaServer.setSipxServiceManager(sipxServiceManager);
    }
}
