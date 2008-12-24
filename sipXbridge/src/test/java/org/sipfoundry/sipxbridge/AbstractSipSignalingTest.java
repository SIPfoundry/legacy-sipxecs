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

import org.cafesip.sipunit.SipTestCase;
import org.sipfoundry.sipxbridge.xmlrpc.SipXbridgeXmlRpcClient;

public abstract class AbstractSipSignalingTest extends SipTestCase {
	protected int sipxProxyPort = 5060;
	protected String localAddr;
	protected int localPort;
	private static int baseMediaPort;
	protected ItspAccountInfo accountInfo;
	protected AccountManagerImpl accountManager;
	protected String sipxProxyAddress;
	protected SipXbridgeXmlRpcClient client;

	@Override
	public void setUp() throws Exception {
		Properties properties = new Properties();
		properties.load(new FileInputStream(new File(
				"testdata/selftest.properties")));
		String accountName = properties
				.getProperty("org.sipfoundry.gateway.mockItspAccount");
		System.out.println("config file name " + accountName);
		Gateway.setConfigurationFileName(accountName);
		Gateway.parseConfigurationFile();

		SipXbridgeXmlRpcServerImpl.startXmlRpcServer();
		Gateway.start();
		System.out.println("Web server started");
		this.client = new SipXbridgeXmlRpcClient(Gateway.getAccountManager()
				.getBridgeConfiguration().getExternalAddress(), Gateway
				.getAccountManager().getBridgeConfiguration().getXmlRpcPort(),
				Gateway.getBridgeConfiguration().isSecure());

		accountManager = Gateway.getAccountManager();
		accountInfo = accountManager.getDefaultAccount();
		sipxProxyAddress = properties
				.getProperty("org.sipfoundry.gateway.mockSipxProxyAddress");
		localAddr = Gateway.getAccountManager().getBridgeConfiguration()
				.getLocalAddress();
		localPort = Gateway.getAccountManager().getBridgeConfiguration()
				.getLocalPort();
		baseMediaPort = Integer.parseInt(properties
				.getProperty("org.sipfoundry.gateway.mediaBasePort"));
	}

	@Override
	public void tearDown() throws Exception {
		Gateway.stop();
	}

	public int getMediaPort() {
		return baseMediaPort++;
	}

}
