package org.sipfoundry.sipxbridge;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.net.URL;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;
import org.sipfoundry.sipxbridge.Gateway;
import org.sipfoundry.sipxbridge.GatewayState;

import junit.framework.TestCase;

public class InvalidAccountTest extends TestCase {

    private static Logger logger = Logger.getLogger(InvalidAccountTest.class);
    private static String serverAddress;
    private static  int port ;
    private XmlRpcClient client;

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
       
        Gateway.startXmlRpcServer();
        port = Gateway.getAccountManager().getBridgeConfiguration().getXmlRpcPort();
        serverAddress = Gateway.getAccountManager().getBridgeConfiguration().getExternalAddress();
        XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();
        config.setServerURL(new URL("http://" + serverAddress + ":" + port));
        client = new XmlRpcClient();
        client.setConfig(config);

    }

    public void testBadAccountInfo() {
        try {
            String retval = (String) client.execute("sipXbridge.start",
                    (Object[]) null);
            assertTrue("State must be INITIALIZING", retval
                    .equals(GatewayState.STOPPED.toString()));

        } catch (XmlRpcException e) {
            e.printStackTrace();
            logger.error("Unexpected exeption ", e);
            fail("Unexpected exception");

        }
    }

    protected void tearDown() throws Exception {
        super.tearDown();
        client.execute("sipXbridge.stop", (Object[]) null);
        Gateway.stopXmlRpcServer();
    }

}
