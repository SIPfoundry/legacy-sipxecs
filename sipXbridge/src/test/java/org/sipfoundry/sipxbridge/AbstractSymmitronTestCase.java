package org.sipfoundry.sipxbridge;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.net.URL;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.PropertyConfigurator;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;

import junit.framework.TestCase;

public abstract class AbstractSymmitronTestCase extends TestCase {
    protected static  String serverAddress ;
    protected static int port ; // Make sure your sever is running there.
    protected XmlRpcClient client;
    protected static String clientHandle = "nat:12345";
    
    

    /*
     * (non-Javadoc)
     * 
     * @see junit.framework.TestCase#setUp()
     */

    protected void setUp() throws Exception {
       
        super.setUp();
        Properties properties = new Properties();
        properties.load(new FileInputStream ( new File ( "testdata/selftest.properties")));
        String accountName = properties.getProperty("org.sipfoundry.gateway.noItspAccount");
        port = Integer.parseInt(properties.getProperty("org.sipfoundry.gateway.xmlRpcPort"));
        serverAddress = properties.getProperty("org.sipfoundry.gateway.serverAddress");
        Gateway.setConfigurationFileName(accountName);
        Gateway.startXmlRpcServer();
        XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();
        config.setServerURL(new URL("http://" + serverAddress + ":" + port));
        client = new XmlRpcClient();
        client.setConfig(config);

    }
    
    
    protected void tearDown() throws Exception {
        super.tearDown();
        client.execute("sipXbridge.stop", (Object[]) null);
        
    }
    
    protected void checkStandardMap (Map retval) {
        assertTrue(retval.get(Symmitron.STATUS_CODE).equals(Symmitron.OK));
        assertNotNull(retval.get(Symmitron.INSTANCE_HANDLE));
    }
   
    protected void start() throws Exception {
        String retval = (String) client.execute("sipXbridge.start",
                (Object[]) null);
        System.out.println("StartBridge returned " + retval);
    }
    
    protected String createEvenSym() throws Exception {
        int count = 1;
        Object[] args = new Object[3];
        args[0] = clientHandle;
        args[1] = new Integer(count);
        args[2] = Symmitron.EVEN;
        
        Map retval = (Map) client.execute("sipXbridge.createSyms", args);
        
        Object[] syms = (Object[]) retval.get(Symmitron.SYM_SESSION);
        return ( String ) syms[0];
    }
    
    protected String createBridge() throws Exception {
        Object[] args = new Object[1];
        args[0] = this.clientHandle;
        Map retval = (Map) client.execute("sipXbridge.createBridge", args);
        return (String) retval.get(Symmitron.BRIDGE_ID);
    }
    
    protected void destroyBridge(String bridgeId) throws Exception {
        Object[] args = new Object[2];
        args[0] = this.clientHandle;
        args[1] = bridgeId;
        client.execute("sipXbridge.destroyBridge",args);
    }
    
    protected String createOddSym() throws Exception {
        int count = 1;
        Object[] args = new Object[3];
        args[0] = clientHandle;
        args[1] = new Integer(count);
        args[2] = Symmitron.ODD;
        
        Map retval = (Map) client.execute("sipXbridge.createSyms", args);
        
        Object[] syms = (Object[]) retval.get(Symmitron.SYM_SESSION);
        return ( String ) syms[0];
    }
    
    protected void setRemoteEndpoint( String sym, int destinationPort) throws Exception {
        Object[] params = new Object[8];
        params[0] = clientHandle;
        params[1] = sym;
        params[2] = serverAddress;
        params[3] = new Integer(destinationPort);
        params[4] = new Integer(500);
        params[5] = "USE-EMPTY-PACKET";
        params[6] = "";
        params[7] = new Boolean(false);
        client.execute("sipXbridge.setDestination", params);

    }
    
    protected void addSym (  String bridge, String sym)  throws Exception  {
        Object[] params = new Object[3];
        params[0] = clientHandle;
        params[1] = bridge;
        params[2] = sym;
        Map retval = (Map) client.execute("sipXbridge.addSym",params);
    }
    
   
    
    protected void signIn() throws Exception {
        String[] myHandle = new String[1] ;
        myHandle[0] = clientHandle;
        Map retval = (Map) client.execute("sipXbridge.signIn",
                (Object[]) myHandle);
    }
}
