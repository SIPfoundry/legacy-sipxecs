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

import javax.sip.header.ServerHeader;
import javax.sip.header.UserAgentHeader;

import org.cafesip.sipunit.SipTestCase;
import org.sipfoundry.sipxrelay.SymmitronServer;
import org.sipfoundry.sipxbridge.xmlrpc.SipXbridgeXmlRpcClient;

public abstract class AbstractSipSignalingTest extends SipTestCase {
	protected int sipxProxyPort = 5060;
	protected String localAddr;
	protected int localPort;
	private static int baseMediaPort;
	protected ItspAccountInfo accountInfo;
	protected AccountManagerImpl accountManager;
	protected static String sipxProxyAddress;

	static {
		try {

			Properties properties = new Properties();
			properties.load(new FileInputStream(new File(
					"testdata/selftest.properties")));
			String accountDir = properties
			.getProperty("org.sipfoundry.gateway.mockItspAccount");
			System.out.println("accountdir = " + accountDir);
			System.setProperty("conf.dir", accountDir);
			sipxProxyAddress = properties
			.getProperty("org.sipfoundry.gateway.mockSipxProxyAddress");
			System.setProperty("conf.dir", accountDir);
			baseMediaPort = Integer.parseInt(properties
					.getProperty("org.sipfoundry.gateway.mediaBasePort"));

			SymmitronServer.start(); /* Starting the symmitron only once for these tests */
			System.out.println("SymmitronServer -- started.");
		} catch ( Exception ex) {
			ex.printStackTrace();
			throw new RuntimeException(ex);
		}
	}

	@Override
	public void setUp() throws Exception {


		Gateway.parseConfigurationFile();
		accountManager = Gateway.getAccountManager();
		accountInfo = accountManager.getDefaultAccount();
		localAddr = Gateway.getAccountManager().getBridgeConfiguration()
				.getLocalAddress();
		localPort = Gateway.getAccountManager().getBridgeConfiguration()
				.getLocalPort();
		Gateway.initializeLogging();

	}

	@Override
	public void tearDown() throws Exception {
		Gateway.stop();
		SymmitronServer.stop();

	}

	public int getMediaPort() {
		return baseMediaPort++;
	}

}
