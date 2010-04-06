/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxbridge;

import junit.framework.TestCase;

import org.sipfoundry.sipxbridge.xmlrpc.RegistrationRecord;
import org.sipfoundry.sipxbridge.xmlrpc.SipXbridgeXmlRpcClient;


public class RegistrationStatusTest extends TestCase {
	SipXbridgeXmlRpcClient client = null;
	public void setUp() {
		System.out.println("Please set up two ITSP accounts with registration before running this test");
		System.out.println("Please turn of secure xml rpc on sipxbridge");
		client = new SipXbridgeXmlRpcClient("192.168.5.75", 8088, false);
	}
	
	public void testRegistrationStatus() {
		RegistrationRecord[] records = client.getRegistrationRecords();
		assertEquals("registrations must be 2" , 2, records.length);
	}

}
