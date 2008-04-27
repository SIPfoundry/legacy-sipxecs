package org.sipfoundry.sipxbridge;

import java.io.File;
import java.io.FileReader;
import java.net.URL;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;

import junit.framework.TestCase;

public class StartStopBridgeTest extends AbstractSymmitronTestCase {
    private static Logger logger = Logger.getLogger(StartStopBridgeTest.class);

  

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

    

}
