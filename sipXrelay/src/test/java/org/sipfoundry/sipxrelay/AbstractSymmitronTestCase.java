/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrelay;

import java.io.File;
import java.io.FileInputStream;
import java.net.URL;
import java.util.Map;
import java.util.Properties;
import java.util.Random;

import junit.framework.TestCase;

import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;
import org.sipfoundry.sipxrelay.SymEndpointImpl;
import org.sipfoundry.sipxrelay.SymImpl;
import org.sipfoundry.sipxrelay.SymInterface;
import org.sipfoundry.sipxrelay.Symmitron;
import org.sipfoundry.sipxrelay.SymmitronConfig;
import org.sipfoundry.sipxrelay.SymmitronConfigParser;
import org.sipfoundry.sipxrelay.SymmitronServer;

public abstract class AbstractSymmitronTestCase extends TestCase {
    protected static String serverAddress;
    protected static int port; // Make sure your sever is running there.
    protected XmlRpcClient client;
    protected String clientHandle;
    protected String testerAddress;

    protected void connectToServer() throws Exception {
        clientHandle = "thruput-tester:" + new Random().nextLong();
        XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();
        config.setServerURL(new URL("http://" + serverAddress + ":" + port));
        client = new XmlRpcClient();
        client.setConfig(config);
    }

    /*
     * 
     * @see junit.framework.TestCase#setUp()
     */

    protected void setUp() throws Exception {

        super.setUp();
        Properties properties = new Properties();
        properties.load(new FileInputStream(new File("testdata/selftest.properties")));
        String accountName = properties.getProperty("org.sipfoundry.gateway.symmitronConfig");
        this.testerAddress = properties.getProperty("org.sipfoundry.gateway.symmitronTestClientAddress");
        SymmitronConfigParser parser = new SymmitronConfigParser();
        String url = "file:" + accountName;

        SymmitronConfig symConfig = parser.parse(url);
        symConfig.setLogLevel("trace");
        SymmitronServer.setSymmitronConfig(symConfig);
        SymmitronServer.startWebServer();
        port = symConfig.getXmlRpcPort();
        serverAddress = symConfig.getLocalAddress();
        this.connectToServer();

    }

    protected void tearDown() throws Exception {
        super.tearDown();
        client.execute("sipXrelay.tearDown", (Object[]) null);

    }

    protected void checkStandardMap(Map retval) {
        assertTrue(retval.get(Symmitron.STATUS_CODE).equals(Symmitron.OK));
        assertNotNull(retval.get(Symmitron.INSTANCE_HANDLE));
    }

    protected String start() throws Exception {
      //  String retval = (String) client.execute("sipXrelay.start", (Object[]) null);
        return "OK";

    }

    protected String createEvenSym() throws Exception {
        int count = 1;
        Object[] args = new Object[3];
        args[0] = clientHandle;
        args[1] = new Integer(count);
        args[2] = Symmitron.EVEN;

        Map retval = (Map) client.execute("sipXrelay.createSyms", args);
        if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " + retval.get(Symmitron.ERROR_INFO));
        }
        Object[] syms = (Object[]) retval.get(Symmitron.SYM_SESSION);
        Map sym = (Map) syms[0];
        return (String) sym.get("id");
    }

    protected String createBridge() throws Exception {
        Object[] args = new Object[1];
        args[0] = this.clientHandle;
        Map retval = (Map) client.execute("sipXrelay.createBridge", args);
        if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " + retval.get(Symmitron.ERROR_INFO));
        }
        return (String) retval.get(Symmitron.BRIDGE_ID);
    }

    protected void destroyBridge(String bridgeId) throws Exception {
        Object[] args = new Object[2];
        args[0] = this.clientHandle;
        args[1] = bridgeId;
        Map retval = (Map) client.execute("sipXrelay.destroyBridge", args);
        if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " + retval.get(Symmitron.ERROR_INFO));
        }
    }

    protected String createOddSym() throws Exception {
        int count = 1;
        Object[] args = new Object[3];
        args[0] = clientHandle;
        args[1] = new Integer(count);
        args[2] = Symmitron.ODD;

        Map retval = (Map) client.execute("sipXrelay.createSyms", args);
        if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " + retval.get(Symmitron.ERROR_INFO));
        }
        Object[] syms = (Object[]) retval.get(Symmitron.SYM_SESSION);
        return (String) syms[0];
    }

    protected void setRemoteEndpoint(String sym, int destinationPort) throws Exception {
        Object[] params = new Object[6];
        params[0] = clientHandle;
        params[1] = sym;
        params[2] = serverAddress;
        params[3] = new Integer(destinationPort);
        params[4] = new Integer(500);
        params[5] = "USE-EMPTY-PACKET";

        Map retval = (Map) client.execute("sipXrelay.setDestination", params);
        if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " + retval.get(Symmitron.ERROR_INFO));
        }

    }
    protected void setRemoteEndpointNoKeepalive(String sym, int destinationPort) throws Exception {
        Object[] params = new Object[6];
        params[0] = clientHandle;
        params[1] = sym;
        params[2] = serverAddress;
        params[3] = new Integer(destinationPort);
        params[4] = new Integer(500);
        params[5] = "NONE";

        Map retval = (Map) client.execute("sipXrelay.setDestination", params);
        if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " + retval.get(Symmitron.ERROR_INFO));
        }

    }
    protected void setRemoteEndpointNoKeepAlive(String sym, int destinationPort) throws Exception {
        Object[] params = new Object[6];
        params[0] = clientHandle;
        params[1] = sym;
        params[2] = serverAddress;
        params[3] = new Integer(destinationPort);
        params[4] = new Integer(500);
        params[5] = "NONE";

        Map retval = (Map) client.execute("sipXrelay.setDestination", params);
        if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " + retval.get(Symmitron.ERROR_INFO));
        }

    }

    protected void setRemoteEndpointNoKeepAlive(String sym, String clientAddress, int destinationPort) throws Exception {
        Object[] params = new Object[6];
        params[0] = clientHandle;
        params[1] = sym;
     
        params[2] = clientAddress;
        params[3] = new Integer(destinationPort);
        params[4] = new Integer(500);
        params[5] = "NONE";

        Map retval = (Map) client.execute("sipXrelay.setDestination", params);
        if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " + retval.get(Symmitron.ERROR_INFO));
        }

    }

    protected void setAutoDiscover(String sym) throws Exception {
        Object[] params = new Object[6];
        params[0] = clientHandle;
        params[1] = sym;
        params[2] = "";
        params[3] = 0;
        params[4] = new Integer(500);
        params[5] = "USE-EMPTY-PACKET";

        Map retval = (Map) client.execute("sipXrelay.setDestination", params);
        if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " + retval.get(Symmitron.ERROR_INFO));
        }

    }

    protected void addSym(String bridge, String sym) throws Exception {
        Object[] params = new Object[3];
        params[0] = clientHandle;
        params[1] = bridge;
        params[2] = sym;
        Map retval = (Map) client.execute("sipXrelay.addSym", params);
        if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " + retval.get(Symmitron.ERROR_INFO));
        }
    }

    protected void removeSym(String bridge, String sym) throws Exception {
        Object[] parms = new Object[3];
        parms[0] = clientHandle;
        parms[1] = bridge;
        parms[2] = sym;
        Map retval = (Map) client.execute("sipXrelay.removeSym", parms);
        if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " + retval.get(Symmitron.ERROR_INFO));
        }

    }

    /**
     * Get a Sym.
     */
    protected SymInterface getSym(String sym) throws Exception {
        Object[] args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym;
        Map retval = (Map) client.execute("sipXrelay.getSym", args);
        if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " + retval.get(Symmitron.ERROR_INFO));
        }
        SymImpl symImpl = new SymImpl();

        Map symSession = (Map) retval.get(Symmitron.SYM_SESSION);
        Map receiverSession = (Map) symSession.get("receiver");
        if (receiverSession != null && !receiverSession.isEmpty()) {
            String ipAddr = (String) receiverSession.get("ipAddress");
            int port = (Integer) receiverSession.get("port");
            String id = (String) receiverSession.get("id");

            SymEndpointImpl receiverEndpoint = new SymEndpointImpl();
            receiverEndpoint.setIpAddress(ipAddr);
            receiverEndpoint.setPort(port);
            receiverEndpoint.setId(id);
            symImpl.setReceiver(receiverEndpoint);
        }

        Map transmitterSession = (Map) symSession.get("transmitter");
        if (transmitterSession != null && !transmitterSession.isEmpty()) {
            String ipAddr = (String) transmitterSession.get("ipAddress");

            int port = (Integer) transmitterSession.get("port");
            String id = (String) transmitterSession.get("id");

            SymEndpointImpl transmitterEndpoint = new SymEndpointImpl();
            transmitterEndpoint.setIpAddress(ipAddr);
            transmitterEndpoint.setPort(port);
            transmitterEndpoint.setId(id);
            symImpl.setTransmitter(transmitterEndpoint);
        }
        return symImpl;
    }

    public void pauseBridge(String bridge) throws Exception {
        Object[] args = new Object[2];
        args[0] = clientHandle;
        args[1] = bridge;
        Map retval = (Map) client.execute("sipXrelay.pauseBridge", args);
        if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " + retval.get(Symmitron.ERROR_INFO));
        }
    }

    public void resumeBridge(String bridge) throws Exception {
        Object[] args = new Object[2];
        args[0] = clientHandle;
        args[1] = bridge;
        Map retval = (Map) client.execute("sipXrelay.resumeBridge", args);
        if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " + retval.get(Symmitron.ERROR_INFO));
        }
    }

    public void startBridge(String bridge) throws Exception {
        Object[] args = new Object[2];
        args[0] = clientHandle;
        args[1] = bridge;
        Map retval = (Map) client.execute("sipXrelay.startBridge", args);
        if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
            throw new Exception("Error in processing request " + retval.get(Symmitron.ERROR_INFO));
        }
    }

    protected void signIn() throws Exception {
        String[] myHandle = new String[1];
        myHandle[0] = clientHandle;
        Map retval = (Map) client.execute("sipXrelay.signIn", (Object[]) myHandle);
    }

    protected void signOut() throws Exception {
        String[] myHandle = new String[1];
        myHandle[0] = clientHandle;
        client.execute("sipXrelay.signOut", (Object[]) myHandle);

    }

    public void destroySym(String sym) throws Exception {
        Object[] args = new Object[2];
        args[0] = clientHandle;
        args[1] = sym;
        Map retval = (Map) client.execute("sipXrelay.destroySym", args);

    }
}
