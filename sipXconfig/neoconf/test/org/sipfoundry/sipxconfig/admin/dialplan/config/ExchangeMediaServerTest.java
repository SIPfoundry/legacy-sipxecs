/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.dialplan.CallDigits;
import org.sipfoundry.sipxconfig.admin.dialplan.ExchangeMediaServer;
import org.sipfoundry.sipxconfig.admin.dialplan.MediaServer.Operation;

public class ExchangeMediaServerTest extends TestCase {

    private ExchangeMediaServer m_out;
    private static final String VOICEMAIL_EXTENSION = "101";
    private static final String HOSTNAME = "exchange.example.com";
    
    public void setUp() {
        m_out = new ExchangeMediaServer();
        m_out.setHostname(HOSTNAME);
        m_out.setServerExtension(VOICEMAIL_EXTENSION);
    }
    
    public void testConstructorWithArgs() {
        ExchangeMediaServer out = new ExchangeMediaServer(HOSTNAME, VOICEMAIL_EXTENSION);
        assertEquals("Wrong value for hostname.", HOSTNAME, out.getHostname());
        assertEquals("Wrong value for extension.", VOICEMAIL_EXTENSION, out.getServerExtension());
    }
    
    public void testGetHeaderParams() {
        assertEquals("Wrong header param string.",
                "Diversion=%3Ctel:{digits}%3E%3Breason%3Dno-answer%3Bscreen%3Dno%3Bprivacy%3Doff",
                m_out.getHeaderParameterStringForOperation(Operation.VoicemailDeposit, 
                        CallDigits.FIXED_DIGITS, null));
    }
    
    public void testGetUriParams() {
        String paramsForRetrieve = m_out.getUriParameterStringForOperation(Operation.VoicemailRetrieve, null, null);
        assertEquals("Wrong uri param string.", 
                "transport=tcp",
                paramsForRetrieve);
    }
    
    public void testGetDigitsForOperation() {
        assertEquals("Wrong digits for voicemail deposit.",
                VOICEMAIL_EXTENSION, m_out.getDigitStringForOperation(
                        Operation.VoicemailDeposit, CallDigits.FIXED_DIGITS));
    }
    
    public void testGetAddress() {
        assertEquals("Wrong server address.", HOSTNAME, m_out.getHostname());
    }
}
