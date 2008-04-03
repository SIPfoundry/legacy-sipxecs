package org.sipfoundry.sipxbridge;

import java.net.URL;

import org.apache.log4j.Logger;
import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;

import junit.framework.TestCase;

public class StartStopBridgeTest extends TestCase {
	private static Logger logger = Logger.getLogger(StartStopBridgeTest.class);

	private static final String serverAddress = "192.168.5.240";
	private static final int port = 8080;
	private XmlRpcClient client;

	/*
	 * (non-Javadoc)
	 * 
	 * @see junit.framework.TestCase#setUp()
	 */

	protected void setUp() throws Exception {
		// WE ASSUME HERE that there is a valid account in
		// /etc/sipxpbx/sipxbridge.xml
		super.setUp();
		Gateway.startXmlRpcServer();
		XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();
		config.setServerURL(new URL("http://" + serverAddress + ":" + port));
		client = new XmlRpcClient();
		client.setConfig(config);

	}

	public void testStartServer() {
		try {
			String retval = (String) client.execute("sipXbridge.start",
					(Object[]) null);
			assertTrue("State must be initialized", retval
					.equals(GatewayState.INITIALIZED.toString()));
			retval = (String) client.execute("sipXbridge.stop",
					((Object[]) null));
			assertTrue("State must be stopped", retval
					.equals(GatewayState.STOPPED.toString()));
			retval = (String) client.execute("sipXbridge.start",
					(Object[]) null);
			assertTrue("State must be initialized", retval
					.equals(GatewayState.INITIALIZED.toString()));

		} catch (XmlRpcException e) {
			e.printStackTrace();
			logger.error("Unexpected exeption ", e);
			fail("Unexpected exception");

		}

	}

	protected void tearDown() throws Exception {
		super.tearDown();
		client.execute("sipXbridge.stop",(Object[]) null);
		Gateway.stopXmlRpcServer();
	}

}
