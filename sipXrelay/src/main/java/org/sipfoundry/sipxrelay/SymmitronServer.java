/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrelay;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Properties;
import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLServerSocket;
import javax.net.ssl.SSLSession;

import net.java.stun4j.StunAddress;
import net.java.stun4j.client.NetworkConfigurationDiscoveryProcess;
import net.java.stun4j.client.StunDiscoveryReport;

import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import org.apache.xmlrpc.XmlRpcException;
import org.mortbay.http.HttpContext;
import org.mortbay.http.HttpListener;
import org.mortbay.http.HttpServer;
import org.mortbay.http.SocketListener;
import org.mortbay.http.SslListener;
import org.mortbay.jetty.servlet.ServletHandler;
import org.mortbay.util.InetAddrPort;
import org.mortbay.util.ThreadedServer;
import org.sipfoundry.commons.alarm.SipXAlarmClient;
import org.sipfoundry.commons.log4j.SipFoundryAppender;
import org.sipfoundry.commons.log4j.SipFoundryLayout;

/**
 * The SIPXbridge XML RPC handler.
 * 
 * @author M. Ranganathan
 * 
 */
@SuppressWarnings("unchecked")
public class SymmitronServer implements Symmitron {

    static Logger logger = Logger.getLogger(SymmitronServer.class.getPackage()
            .getName());

    protected static Timer timer = new Timer();

    private static String status;

    /*
     * Map that pairs the SymSession id with the SymSession
     */
    private static Map<String, Sym> sessionMap = new ConcurrentHashMap<String, Sym>();

    /*
     * Map that pairs the rtpSesion owner with the rtp session
     */
    private static Map<String, HashSet<Sym>> sessionResourceMap = new ConcurrentHashMap<String, HashSet<Sym>>();

    /*
     * A map that pairs the RtpBridge id with the rtp Bridge.
     */
    private static Map<String, Bridge> bridgeMap = new ConcurrentHashMap<String, Bridge>();

    /*
     * A map that pairs the owner with the RtpBridge
     */
    private static Map<String, HashSet<Bridge>> bridgeResourceMap = new ConcurrentHashMap<String, HashSet<Bridge>>();

    /*
     * A map of component name to instance handle.
     */
    private static Map<String, String> instanceTable = new ConcurrentHashMap<String, String>();
    
    /*
     * Ptr to our port range manager.
     */

    private static PortRangeManager portRangeManager;

    private static boolean alarmSent;

    private static boolean addressResolutionAlarmSent = false;

    /*
     * My Instance handle
     */
    private static String myHandle = "sipxbridge:"
            + Math.abs(new Random().nextLong());

    /*
     * True if our web server is running ( the one that handles xml rpc requests ).
     */
    private static boolean isWebServerRunning;

    /*
     * Pointer to our web server.
     */
    private static HttpServer webServer;

    /*
     * Local address by name.
     */
    private static InetAddress localAddressByName;

    /*
     * Public IP address.
     */
    private static InetAddress publicAddress;

    /*
     * Pointer to our configuration.
     */
    static SymmitronConfig symmitronConfig;

    /*
     * The standard STUN port.
     */
    private static final int STUN_PORT = 3478;

    /*
     * Pointer to sipx alarm client.
     */
    static SipXAlarmClient alarmClient;

    /*
     * Dir where config file is stored.
     */
    private static String configDir;

    private static final String STUN_FAILURE_ALARM_ID = "STUN_ADDRESS_DISCOVERY_FAILED";

    private static final String STUN_RECOVERY_ALARM_ID = "STUN_ADDRESS_DISCOVERY_RECOVERED";

    private static final String STUN_ADDRESS_ERROR_ALARM_ID = "STUN_ADDRESS_ERROR";

    static final String SIPX_RELAY_STRAY_PACKET_ALARM_ID = "MEDIA_RELAY_STRAY_PACKETS_DETECTED";

    static final int TIMEOUT = 1000;
    
    private static NetworkConfigurationDiscoveryProcess addressDiscovery = null;
    
    static CRLFReceiver crlfReceiver;

    static boolean filterStrayPackets = true;
  
    private static Thread dataShufflerThread;

    private static DataShuffler dataShuffler;

    
    private static ConcurrentHashMap<String,SipXrelaySemaphore> semaphoreLockTable = new ConcurrentHashMap<String,SipXrelaySemaphore>();

    private static long startTime;
    
    static {
        try {
            crlfReceiver = new CRLFReceiver();
            new Thread(crlfReceiver).start();
        } catch (Exception ex) {
            throw new RuntimeException(ex);
        }
    }
    
    
    /**
     * A work queue item to set the destination. We need this to be a synchronous
     * operation that is done in the data shuffler thread. This is single threaded
     * to avoid locking. This has to be done this way because there are two ways in
     * which the destination can be set -- auto learning or application set. Since
     * these two methods are in different threads, there is a conflict.
     * 
     */ 
    class SetDestinationWorkItem extends WorkItem {

        String ipAddress;
        int port;
        String keepaliveMethod;
        int keepAliveTime;
        Sym sym;
        SipXrelaySemaphore workSem;
    
      
        public SetDestinationWorkItem(Sym sym, String ipAddress, int port, int keepAliveTime, String keepaliveMethod) {
            super();
            if ( port <  0 || keepAliveTime < 0 || keepaliveMethod == null ||
            		KeepaliveMethod.valueOfString(keepaliveMethod) == null ) {
            	throw new IllegalArgumentException(String.format("Bad parameter sym = %s ipaddr = %s port = %d keepaliveTime= %d keepaliveMethod = %s ",
            			sym.getId(), ipAddress,port, keepAliveTime, keepaliveMethod));
            }
            this.workSem = new SipXrelaySemaphore(0);
            this.ipAddress = ipAddress;
            this.port = port;
            this.keepAliveTime = keepAliveTime;
            this.sym = sym;
            this.keepaliveMethod = keepaliveMethod;
        }

        public void doWork() {
            try {
                // Allocate a new session if needed.
                SymTransmitterEndpoint transmitter = sym.getTransmitter() != null ? sym
                        .getTransmitter()
                        : new SymTransmitterEndpoint();

                AutoDiscoveryFlag autoDiscoveryFlag = AutoDiscoveryFlag.NO_AUTO_DISCOVERY;
                if (ipAddress == null && port == 0) {
                    autoDiscoveryFlag = AutoDiscoveryFlag.IP_ADDRESS_AND_PORT;
                } else if (ipAddress != null && port == 0) {
                    autoDiscoveryFlag = AutoDiscoveryFlag.PORT_ONLY;
                }
                transmitter.setAutoDiscoveryFlag(autoDiscoveryFlag);

                transmitter.setIpAddressAndPort(ipAddress, port);

                KeepaliveMethod method = KeepaliveMethod
                        .valueOfString(keepaliveMethod);
                transmitter.setMaxSilence(keepAliveTime, method);

                if (sym.getTransmitter() == null) {
                    sym.setTransmitter(transmitter);
                }
               
            } catch (Exception ex) {
                logger.error("Exception setting destination ", ex);
                super.error(PROCESSING_ERROR,ex.getMessage());
                
            } finally {
                workSem.release();
            }
        }
    }

    /**
     * A work queue item to add a sym to a bridge. Dont want to add syms to a bridge when
     * a packet is being processed.
     * 
     */ 
    class AddSymWorkItem extends WorkItem {

        String bridgeId;
        String symId;
        SipXrelaySemaphore workSem;
      
        public AddSymWorkItem(String bridgeId, String symId) {
            super();
            this.bridgeId = bridgeId;
            this.symId = symId;
            this.workSem = new SipXrelaySemaphore(0);
        }

        public void doWork() {
            try {
                Bridge bridge = bridgeMap.get(bridgeId);
                
                if (bridge == null) {
                    super.error(SESSION_NOT_FOUND,"Specified bridge was not found " + bridgeId);
                    return;
                }

                Sym sym = sessionMap.get(symId);

                if (sym == null) {
                    super.error(SESSION_NOT_FOUND,
                            "Specified sym was not found " + symId);
                    return;
                }

                bridge.addSym(sym);
            } catch (Exception ex) {
                logger.error("Exception adding sym ", ex);
                super.error(PROCESSING_ERROR,ex.getMessage());
            } finally {
                workSem.release();
            }
        }
    }
    /**
     * A work queue item to add a sym to a bridge. Dont want to add syms to a bridge when
     * a packet is being processed.
     * 
     */ 
    class RemoveSymWorkItem extends WorkItem {

        String bridgeId;
        String symId;
        SipXrelaySemaphore workSem;
      
        public RemoveSymWorkItem(String bridgeId, String symId) {
            super();
            this.bridgeId = bridgeId;
            this.symId = symId;
            this.workSem = new SipXrelaySemaphore(0);
        }

        public void doWork() {
            try {
                Bridge bridge = bridgeMap.get(bridgeId);
                
                if (bridge == null) {
                    super.error(SESSION_NOT_FOUND,"Specified bridge was not found " + bridgeId);
                    return;
                }

                Sym sym = sessionMap.get(symId);

                if (sym == null) {
                    super.error(SESSION_NOT_FOUND,
                            "Specified sym was not found " + symId);
                    return;
                }

                bridge.removeSym(sym);
            } catch (Exception ex) {
                logger.error("Exception adding sym ", ex);
                super.error(PROCESSING_ERROR,ex.getMessage());
            } finally {
                workSem.release();
            }
        }
    }
    
    class GetBridgeStatisticsWorkItem extends WorkItem {
        
        SipXrelaySemaphore workSem = new SipXrelaySemaphore(0);
        
        Map<String,Object> retval ;
        
        String bridgeId;
        
        public GetBridgeStatisticsWorkItem(String bridgeId) {
            this.bridgeId = bridgeId;
        }
        
        public void doWork() {
            try {
                Bridge bridge = bridgeMap.get(bridgeId);
                if ( bridge == null ) {
                    super.error(SESSION_NOT_FOUND,"Specified bridge " + bridgeId + " not found.");
                    return;
                }
                retval = createSuccessMap();
                retval.put(Symmitron.BRIDGE_STATE, bridge.getState().toString());
                retval.put(Symmitron.CREATION_TIME, new Long(bridge
                        .getCreationTime()).toString());
                retval.put(Symmitron.LAST_PACKET_RECEIVED, new Long(bridge
                        .getLastPacketTime()).toString());
                retval.put(Symmitron.CURRENT_TIME_OF_DAY, new Long(System
                        .currentTimeMillis()).toString());
                retval.put(Symmitron.PACKETS_RECEIVED, new Long(
                        bridge.pakcetsReceived).toString());
                retval.put(Symmitron.PACKETS_SENT, new Long(bridge.packetsSent)
                .toString());
                Map[] symStats = new Map[bridge.getSyms().size()];
                int i = 0;
                for (Sym sym : bridge.getSyms()) {
                    symStats[i++] = sym.getStats();
                }
                retval.put(Symmitron.SYM_SESSION_STATS, symStats);
                logger.debug("bridgeStats = " + retval);
            } catch (Exception ex) {
               logger.error("Exception adding sym ", ex);
               super.error(PROCESSING_ERROR,ex.getMessage());
           } finally {
               workSem.release();
           }
        }
    }
    
    private static void restartCrLfReceiver() {
        try {
            crlfReceiver = new CRLFReceiver();
            new Thread(crlfReceiver).start();
        } catch (Exception ex) {
            throw new RuntimeException(ex);
        }
    }

    /**
     * Discover our address using stun.
     * 
     * @throws SipXbridgeException
     */
    static void discoverAddress() throws Exception {
       
        try {

            String stunServerAddress = symmitronConfig.getStunServerAddress();

            if (stunServerAddress != null) {
              
              
                if ( addressDiscovery == null ) { 
                    int localStunPort = STUN_PORT + 1;
                    StunAddress localStunAddress = new StunAddress(symmitronConfig
                            .getLocalAddress(), localStunPort);

                    StunAddress serverStunAddress = new StunAddress(
                            stunServerAddress, STUN_PORT);

                    addressDiscovery = new NetworkConfigurationDiscoveryProcess(
                            localStunAddress, serverStunAddress);
                    java.util.logging.LogManager logManager = java.util.logging.LogManager
                    .getLogManager();
                    java.util.logging.Logger log = logManager
                    .getLogger(NetworkConfigurationDiscoveryProcess.class
                            .getName());
                    log.setLevel(java.util.logging.Level.OFF);
                }

                addressDiscovery.start();
                StunDiscoveryReport report = addressDiscovery
                        .determineAddress();
                if (report == null) {    
                    if (addressDiscovery != null) {
                        try {
                            addressDiscovery.shutDown();
                        } catch (Exception e ) {
                            logger.error("Problem shutting down address discovery!",e);
                        } finally {
                            addressDiscovery = null;
                        }
                    }
                    logger.warn("Error discovering  address -- Check Stun Server");
                    return;
                }
                StunAddress stunAddress = report.getPublicAddress();
                if (stunAddress == null) {
                    logger
                            .error("No stun address - could not do address discovery");
                    return;
                }

                if (publicAddress == null
                        || !publicAddress.equals(stunAddress.getSocketAddress()
                                .getAddress())) {
                    publicAddress = stunAddress.getSocketAddress().getAddress();
                    logger.debug("Public Address Changed! Stun report = "
                            + report);
                    String publicAddr = publicAddress.getHostAddress();
                    logger.debug("publicAddress = " + publicAddr);
                    symmitronConfig.setPublicAddress(publicAddr);
                }

            } else {
                logger.error("Stun server address not specified");
            }
        } catch (Exception ex) {
            if (addressDiscovery != null) {
                try {
                    addressDiscovery.shutDown();
                } catch (Exception e ) {
                    logger.error("Problem shutting down address discovery!",e);
                } finally {
                    addressDiscovery = null;
                }
            }
            logger.error("Error discovering  address -- Check Stun Server", ex);
            return;
        } finally {
           logger.debug("public address = " + publicAddress);
        }
    }
    
    

    private Map<String, Object> createErrorMap(int errorCode, String reason) {
        Map<String, Object> retval = new HashMap<String, Object>();
        retval.put(STATUS_CODE, ERROR);
        retval.put(ERROR_CODE, new Integer(errorCode).toString());
        retval.put(ERROR_INFO, reason);

        retval.put(INSTANCE_HANDLE, myHandle);

        return retval;
    }

    private Map<String, Object> createSuccessMap() {
        Map<String, Object> retval = new HashMap<String, Object>();
        retval.put(STATUS_CODE, OK);
        retval.put(INSTANCE_HANDLE, myHandle);
        return retval;
    }

    private void addSymResource(String controllerId, Sym sym) {
        sessionMap.put(sym.getId(), sym);
        HashSet<Sym> syms = sessionResourceMap.get(controllerId);
        if (syms != null) {
            syms.add(sym);
        } else {
            syms = new HashSet<Sym>();
            syms.add(sym);
            sessionResourceMap.put(controllerId, syms);
        }
    }

    private static InetAddress findIpAddress(String localAddress)
            throws UnknownHostException {
        String[] addressParts = localAddress.split("\\.");
        boolean isIpAddress = true;
        byte[] addressBytes = new byte[addressParts.length];
        int i = 0;
        for (String addressPart : addressParts) {
            try {
                addressBytes[i++] = (byte) Integer.parseInt(addressPart);
            } catch (NumberFormatException ex) {
                isIpAddress = false;
                break;
            }
        }

        InetAddress retval;
        /*
         * User specified a host name -- see if we can resolve it.
         */
        if (!isIpAddress) {
            retval = InetAddress.getByName(localAddress);
        } else {
            /*
             * User specified an address directly -- see if we can resolve that.
             */
            retval = InetAddress.getByAddress(addressBytes);
        }
        return retval;
    }

    public static String getConfigDir() {
        return SymmitronServer.configDir;
    }

    /**
     * Get the port range manager.
     * 
     */
    public static PortRangeManager getPortManager() {
        return portRangeManager;
    }

    public static String getLocalAddress() {
        return symmitronConfig.getLocalAddress();
    }

    public static InetAddress getLocalInetAddress() throws UnknownHostException {
        if (SymmitronServer.localAddressByName == null) {
            SymmitronServer.localAddressByName = InetAddress
                    .getByName(getLocalAddress());
        }
        return localAddressByName;
    }

    public static int getRangeLowerBound() {
        return symmitronConfig.getPortRangeLowerBound();
    }

    public static int getRangeUpperBound() {

        return symmitronConfig.getPortRangeUpperBound();
    }

    public static void setSymmitronConfig(SymmitronConfig symmitronConfig)
            throws IOException {
        SymmitronServer.symmitronConfig = symmitronConfig;
        if (symmitronConfig.getPublicAddress() != null) {
            publicAddress = InetAddress.getByName(symmitronConfig
                    .getPublicAddress());
        }

        portRangeManager = new PortRangeManager(symmitronConfig
                .getPortRangeLowerBound(), symmitronConfig
                .getPortRangeUpperBound());
        
        filterStrayPackets = symmitronConfig.isRejectStrayPackets();
    }

    public static void startWebServer() throws Exception {

        if (!isWebServerRunning) {
            Logger log = Logger.getLogger("org.mortbay");
            log.setLevel(Level.OFF);
            isWebServerRunning = true;
            webServer = new HttpServer();

            InetAddrPort inetAddrPort = new InetAddrPort(symmitronConfig
                    .getLocalAddress(), symmitronConfig.getXmlRpcPort());
            inetAddrPort.setInetAddress(InetAddress.getByName(symmitronConfig
                    .getLocalAddress()));

            if (symmitronConfig.getUseHttps()) {
                logger.info("Starting  secure xml rpc server on inetAddr:port "
                        + symmitronConfig.getLocalAddress() + ":"
                        + symmitronConfig.getXmlRpcPort());

                SslListener sslListener = new SslListener(inetAddrPort);
                inetAddrPort.setInetAddress(InetAddress
                        .getByName(symmitronConfig.getLocalAddress()));

                String keystore = System.getProperties().getProperty(
                        "javax.net.ssl.keyStore");
                logger.info("keystore = " + keystore);
                sslListener.setKeystore(keystore);
                String keystoreType = System
                        .getProperty("javax.net.ssl.trustStoreType");
                logger.info("keyStoreType = " + keystoreType);
                sslListener.setKeystoreType(keystoreType);

                String algorithm = System.getProperties().getProperty(
                        "jetty.x509.algorithm");
                logger.info("algorithm = " + algorithm);
                sslListener.setAlgorithm(algorithm);
                String password = System.getProperties().getProperty(
                        "jetty.ssl.password");
                sslListener.setPassword(password);

                String keypassword = System.getProperties().getProperty(
                        "jetty.ssl.keypassword");

                sslListener.setKeyPassword(keypassword);

                sslListener.setMaxThreads(32);
                sslListener.setMinThreads(4);
                sslListener.setLingerTimeSecs(30000);

                ((ThreadedServer) sslListener).open();

                String[] cypherSuites = ((SSLServerSocket) sslListener
                        .getServerSocket()).getSupportedCipherSuites();

                for (String suite : cypherSuites) {
                    logger.info("Cypher Suites enabled : " + suite);
                }

                ((SSLServerSocket) sslListener.getServerSocket())
                        .setEnabledCipherSuites(cypherSuites);

                String[] protocols = ((SSLServerSocket) sslListener
                        .getServerSocket()).getSupportedProtocols();

                for (String protocol : protocols) {
                    logger.info("Supported protocol = " + protocol);
                }

                ((SSLServerSocket) sslListener.getServerSocket())
                        .setEnabledProtocols(protocols);

                webServer.setListeners(new HttpListener[] { sslListener });

                for (HttpListener listener : webServer.getListeners()) {
                    logger.debug("Listener = " + listener);

                    listener.start();
                }

            } else {
                logger
                        .info("Starting unsecure xml rpc server on inetAddr:port "
                                + symmitronConfig.getLocalAddress()
                                + ":"
                                + symmitronConfig.getXmlRpcPort());

                SocketListener socketListener = new SocketListener(inetAddrPort);
                socketListener.setMaxThreads(32);
                socketListener.setMinThreads(4);
                socketListener.setLingerTimeSecs(30000);
                webServer.addListener(socketListener);
            }

            HttpContext httpContext = new HttpContext();

            httpContext.setContextPath("/");
            ServletHandler servletHandler = new ServletHandler();
            servletHandler.addServlet("symmitron", "/*", SymmitronServlet.class
                    .getName());
            httpContext.addHandler(servletHandler);

            webServer.addContext(httpContext);

            webServer.start();

            logger.debug("Web server started.");

        }
    }

    /**
     * The RPC handler for sipxbridge.
     */
    public SymmitronServer() throws Exception {

    }

    /**
     * Current bridge State.
     * 
     * @return the current bridge state.
     */
    public String getStatus() {
        return status;
    }

    /**
     * Stop the bridge
     */
    public static void stop() {
        logger.info("stop");
        for (Bridge bridge : ConcurrentSet.getBridges()) {
            bridge.stop();
        }

        timer.purge();

        status = "STOPPED";

    }

    /**
     * For testing purposes only
     */
    public Map tearDown() {
        stop();
        return createSuccessMap();
    }

    /**
     * Get the Port Range that is handled by the Bridge.
     * 
     * @return the port range supported by the bridge.
     */

    public Map getRtpPortRange() {

        PortRange portRange = new PortRange(symmitronConfig
                .getPortRangeLowerBound(), symmitronConfig
                .getPortRangeUpperBound());

        return portRange.toMap();

    }

    public static InetAddress getPublicInetAddress() {
        return publicAddress;
    }

    /**
     * Check for client reboot.
     * 
     * @param controllerHandle -
     *            a client handle in the form componentName:instanceId
     */
    public void checkForControllerReboot(String controllerHandle) throws Exception {
        String[] handleParts = controllerHandle.split(":");
        if (handleParts.length != 2) {
            throw new IllegalArgumentException("Illegal handle format");
        }
        String componentName = handleParts[0];
       
        String previousInstance = instanceTable.get(componentName);
        if (previousInstance == null) {
            instanceTable.put(componentName, controllerHandle);
            semaphoreLockTable.put(controllerHandle, new SipXrelaySemaphore(1));
        } else if (!previousInstance.equals(controllerHandle)) {
        	SipXrelaySemaphore previousSem = semaphoreLockTable.get(previousInstance);
        	if ( ! previousSem.tryAcquire(1000, TimeUnit.MILLISECONDS) ) {
        		// guard against pending operations from last generation.
        		throw new Exception("Unable to acquire sem from previous handleInstance");
        	}
            HashSet<Bridge> rtpBridges = bridgeResourceMap
            .get(previousInstance);
            if (rtpBridges != null) {
                for (Bridge rtpBridge : rtpBridges) {
                    rtpBridge.stop();
                }
            }
            bridgeResourceMap.remove(previousInstance);
            bridgeResourceMap.put(controllerHandle, new HashSet<Bridge>());
            HashSet<Sym> rtpSessions = sessionResourceMap.get(previousInstance);
            if (rtpSessions != null) {
                for (Sym rtpSession : rtpSessions) {
                    rtpSession.close();
                }
            }

            semaphoreLockTable.remove(previousInstance);
            sessionResourceMap.remove(previousInstance);
            sessionResourceMap.put(controllerHandle, new HashSet<Sym>());
            semaphoreLockTable.put(controllerHandle, new SipXrelaySemaphore(1));

            instanceTable.put(componentName, controllerHandle);
        }
       
        try {
           if (! this.semaphoreLockTable.get(controllerHandle).tryAcquire(10,TimeUnit.SECONDS) ) {
               logger.error("Error occured during lock acquire for controller handle " + controllerHandle);
               throw new RuntimeException("Could not successfully acquire handle lock for 10 seconds, giving up");
           } else {
               logger.debug("Acquired handle lock for " + controllerHandle);
           }
        } catch (InterruptedException ex) {
            throw new RuntimeException("Interrupted while trying to aquire lock", ex);
        }
        
    }
    
    private void release(String handle) {
        this.semaphoreLockTable.get(handle).release();
    }

    /**
     * Sign in a controller.
     */
    public Map<String, Object> signIn(String remoteHandle) {

        logger.info("signIn " + remoteHandle);
        long currentTime = System.currentTimeMillis();
        if (currentTime - SymmitronServer.startTime < 3000 ) {
            return createErrorMap(PROCESSING_ERROR,"Initialization in progress");
        }
        try {
            checkForControllerReboot(remoteHandle);
            return createSuccessMap();
        } catch (Exception ex) {
            logger.error("Error occured during processing ", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        } finally {
            release(remoteHandle);
        }

    }

    public Map<String, Object> getPublicAddress(String controllerHandle) {
        try {
            checkForControllerReboot(controllerHandle);
            Map<String, Object> retval = createSuccessMap();
            logger.debug(controllerHandle + " getPublicAddress : "
                    + symmitronConfig.getPublicAddress());
            if (symmitronConfig.getPublicAddress() != null) {
                retval.put(PUBLIC_ADDRESS, symmitronConfig.getPublicAddress());
                return retval;
            } else {
                return createErrorMap(PROCESSING_ERROR,
                        "SIXPRELAY Unable to resolve public address using STUN to "
                                + symmitronConfig.getStunServerAddress());
            }
        } catch (Exception ex) {
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        } finally {
            release(controllerHandle);
        }
    }

    /*
     * (non-Javadoc)
     * 
     * @see org.sipfoundry.sipxbridge.Symmitron#createSyms(java.lang.String,
     *      int, int)
     */
    public Map<String, Object> createSyms(String controllerHandle, int count,
            int parity) {
        try {
            this.checkForControllerReboot(controllerHandle);

            PortRange portRange = SymmitronServer.getPortManager().allocate(
                    count, parity == ODD ? Parity.ODD : Parity.EVEN);
            if (portRange == null) {
                logger.error("Ports not available " + count + " parity "
                        + parity);
                return createErrorMap(PORTS_NOT_AVAILABLE,
                        "Ports not available");
            }

            Map<String, Object> retval = createSuccessMap();
            HashMap[] hmapArray = new HashMap[count];
            for (int i = 0; i < count; i++) {
                Sym sym = new Sym();
                SymReceiverEndpoint rtpEndpoint = new SymReceiverEndpoint(
                        portRange.getLowerBound() + i);
                sym.setReceiver(rtpEndpoint);
                hmapArray[i] = sym.toMap();
                this.addSymResource(controllerHandle, sym);
                logger.debug(controllerHandle + " createSym : " + sym.getId());
            }

            retval.put(SYM_SESSION, hmapArray);

            return retval;
        } catch (Exception ex) {
            logger.error("error creating syms", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());

        } finally {
            release(controllerHandle);
        }

    }

    /*
     * (non-Javadoc)
     * 
     * @see org.sipfoundry.sipxbridge.Symmitron#getSym(java.lang.String,
     *      java.lang.String)
     */
    public Map<String, Object> getSym(String controllerHandle, String symId) {

        logger.info("getSym : " + controllerHandle + " symId = " + symId);
        try {
            this.checkForControllerReboot(controllerHandle);

            if (sessionMap.get(symId) != null) {
                Map<String, Object> retval = createSuccessMap();
                Sym sym = sessionMap.get(symId);
                retval.put(SYM_SESSION, sym.toMap());
                logger.debug("returning " + retval);
                return retval;

            } else {
                return createErrorMap(SESSION_NOT_FOUND, "");

            }
        } catch (Exception ex) {
            logger.error("Error processing request " + symId);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        } finally {
            release(controllerHandle);
        }
    }

    /*
     * (non-Javadoc)
     * 
     * @see org.sipfoundry.sipxbridge.Symmitron#setDestination(java.lang.String,
     *      java.lang.String, java.lang.String, int , int, java.lang.String,
     *      byte[], boolean)
     */
    public Map<String, Object> setDestination(String controllerHandle,
            String symId, String ipAddress, int port, int keepAliveTime,
            String keepaliveMethod) {
        try {
            this.checkForControllerReboot(controllerHandle);

         
             logger.info(String.format("setDestination : "
                        + " controllerHande %s " + " symId %s "
                        + " ipAddress %s " + " port %d "
                        + " keepAliveMethod = %s " + " keepAliveTime %d ",
                        controllerHandle, symId, ipAddress, port,
                        keepaliveMethod, keepAliveTime));
         

            Sym sym = sessionMap.get(symId);
            if (sym == null) {
                return createErrorMap(SESSION_NOT_FOUND, "");
            }

            // Check input arguments.
            if (ipAddress.equals("") && port != 0) {
                return createErrorMap(ILLEGAL_ARGUMENT,
                        "Must specify IP address if port is not zero");
            }
            if (ipAddress.equals(""))
                ipAddress = null;

         
            
         
            SetDestinationWorkItem workItem = new SetDestinationWorkItem(sym,ipAddress,port,keepAliveTime,keepaliveMethod);
           
            
            DataShuffler.addWorkItem(workItem);
            
            boolean acquired = workItem.workSem.tryAcquire(TIMEOUT, TimeUnit.MILLISECONDS);
            
            logger.debug("tryAquire returned with value " + acquired);
            if (!acquired ) {
            	return createErrorMap(PROCESSING_ERROR,"SipXrelaySemaphore timed out");
            }
            
            if ( workItem.error ) {
                return createErrorMap(PROCESSING_ERROR, workItem.reason);
            } else return createSuccessMap();

           
        } catch (Exception ex) {

            logger.error("Processing Error", ex);

            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        } finally {
            release(controllerHandle);
        }

    }

    public Map<String, Object> startBridge(String controllerHandle,
            String bridgeId) {
        try {
            logger.info("startBridge: " + controllerHandle + " bridgeId = "
                    + bridgeId);
            this.checkForControllerReboot(controllerHandle);

            Bridge rtpBridge = bridgeMap.get(bridgeId);
            if (rtpBridge == null) {
                return createErrorMap(SESSION_NOT_FOUND, "");
            }
            rtpBridge.start();
            return createSuccessMap();
        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }finally {
            release(controllerHandle);
        }
    }

    public Map<String, Object> pauseSym(String controllerHandle,
            String sessionId) {
        try {
            logger.info("pauseSym: " + controllerHandle + " symId = "
                    + sessionId);
            this.checkForControllerReboot(controllerHandle);
            Sym rtpSession = sessionMap.get(sessionId);
            if (rtpSession == null) {
                return this.createErrorMap(SESSION_NOT_FOUND,
                        "Specified RTP Session was not found " + sessionId);
            }
            if (rtpSession.getTransmitter() == null) {
                return this.createErrorMap(ILLEGAL_STATE,
                        "transmitter is not assigned for rtp session "
                                + sessionId);
            }
            rtpSession.getTransmitter().setOnHold(true);
            return this.createSuccessMap();
        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }finally {
            release(controllerHandle);
        }
    }

    /*
     * (non-Javadoc)
     * 
     * @see org.sipfoundry.sipxbridge.Symmitron#removeSym(java.lang.String,
     *      java.lang.String, java.lang.String)
     */
    public Map<String, Object> removeSym(String controllerHandle,
            String bridgeId, String symId) {
        try {
            logger.info("removeSym: " + controllerHandle + " bridgeId = "
                    + bridgeId + " symId = " + symId);

            this.checkForControllerReboot(controllerHandle);
            Bridge rtpBridge = bridgeMap.get(bridgeId);
            if (rtpBridge == null) {
                return this.createErrorMap(SESSION_NOT_FOUND,
                        "Specified RTP Bridge was not found " + bridgeId);
            }
            
            Bridge bridge = bridgeMap.get(bridgeId);
            
            if (bridge == null) {
                return createErrorMap(SESSION_NOT_FOUND,"Specified bridge was not found " + bridgeId);
            }

            Sym sym = sessionMap.get(symId);

            if (sym == null) {
                return createErrorMap(SESSION_NOT_FOUND,
                        "Specified sym was not found " + symId);
            }
            

            RemoveSymWorkItem removeSym = new RemoveSymWorkItem(bridgeId,symId);
            DataShuffler.addWorkItem(removeSym);
            boolean acquired = removeSym.workSem.tryAcquire(TIMEOUT,TimeUnit.MILLISECONDS);
            if (!acquired) {
            	return createErrorMap(PROCESSING_ERROR,"SipXrelaySemaphore timed out");
            }
            if ( removeSym.error ) {
                return this.createErrorMap(removeSym.errorCode, removeSym.reason);
            } else {
                return this.createSuccessMap();
            }
           
        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }finally {
            release(controllerHandle);
        }
    }

    /*
     * (non-Javadoc)
     * 
     * @see org.sipfoundry.sipxbridge.Symmitron#addSym(java.lang.String,
     *      java.lang.String, java.lang.String)
     */
    public Map<String, Object> addSym(String controllerHandle, String bridgeId,
            String symId) {
        try {
            this.checkForControllerReboot(controllerHandle);
            logger.debug("addSym: " + controllerHandle + " bridgeId = "
                    + bridgeId + " symId = " + symId);
            Bridge bridge = bridgeMap.get(bridgeId);

            if (bridge == null) {
                return createErrorMap(SESSION_NOT_FOUND,"Specified bridge was not found " + bridgeId);

            }

            AddSymWorkItem workItem  = new AddSymWorkItem(bridgeId,symId);
            DataShuffler.addWorkItem(workItem);
            boolean acquired = workItem.workSem.tryAcquire(TIMEOUT,TimeUnit.MILLISECONDS);
            if ( !acquired ) {
                logger.error("addSym: could not acquire sem");
                return this.createErrorMap(PROCESSING_ERROR,"SipXrelaySemaphore timed out");
            } else {
                logger.debug("addSym : acquired sem");
            }
            if ( workItem.error ) {
                return this.createErrorMap(workItem.errorCode, workItem.reason);
            } else {
                return this.createSuccessMap();
            }

        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }finally {
            release(controllerHandle);
        }
    }

    /*
     * (non-Javadoc)
     * 
     * @see org.sipfoundry.sipxbridge.Symmitron#createBridge(java.lang.String,
     *      boolean)
     */
    public Map<String, Object> createBridge(String controllerHandle) {
        try {
            this.checkForControllerReboot(controllerHandle);
            logger.info("createBridge: " + controllerHandle);

            Bridge bridge = new Bridge();
            bridgeMap.put(bridge.getId(), bridge);
            HashSet<Bridge> bridges = bridgeResourceMap.get(controllerHandle);
            if (bridges != null) {
                bridges.add(bridge);
            } else {
                bridges = new HashSet<Bridge>();
                bridges.add(bridge);
                bridgeResourceMap.put(controllerHandle, bridges);
            }

            Map<String, Object> retval = this.createSuccessMap();
            logger.debug("createBridge: returning " + bridge.getId());
            retval.put(BRIDGE_ID, bridge.getId());
            return retval;
        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }finally {
            release(controllerHandle);
        }

    }

    public Map<String, Object> pauseBridge(String controllerHandle,
            String bridgeId) {
        try {
            logger.info(" pauseBridge: " + controllerHandle + " " + bridgeId);
            this.checkForControllerReboot(controllerHandle);
            Bridge bridge = bridgeMap.get(bridgeId);
            if (bridge == null) {
                return this.createErrorMap(SESSION_NOT_FOUND,
                        "Bridge corresponding to " + bridgeId + " not found");
            }
            bridge.pause();
            return this.createSuccessMap();
        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }finally {
            release(controllerHandle);
        }
    }

    public Map<String, Object> resumeBridge(String controllerHandle,
            String bridgeId) {
        try {
            logger.info("resumeBridge : " + controllerHandle + " bridgeId " + bridgeId);
            this.checkForControllerReboot(controllerHandle);
            Bridge rtpBridge = bridgeMap.get(bridgeId);
            if (rtpBridge == null) {
                return this.createErrorMap(SESSION_NOT_FOUND,
                        "Bridge corresponding to " + bridgeId + " not found");
            }
            rtpBridge.resume();
            return this.createSuccessMap();
        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }finally {
            release(controllerHandle);
        }
    }

    public Map<String, Object> resumeSym(String controllerHandle,
            String sessionId) {
        try {
            logger.info("resumeSym : " + controllerHandle + " symId = "
                    + sessionId);
            this.checkForControllerReboot(controllerHandle);
            Sym rtpSession = sessionMap.get(sessionId);
            if (rtpSession == null) {
                return this.createErrorMap(SESSION_NOT_FOUND,
                        "Specified sym was not found " + sessionId);
            }
            if (rtpSession.getTransmitter() == null) {
                return this.createErrorMap(ILLEGAL_STATE,
                        "transmitter is not assigned for rtp session "
                                + sessionId);
            }
            rtpSession.getTransmitter().setOnHold(false);
            return this.createSuccessMap();
        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }finally {
            release(controllerHandle);
        }
    }

    public Map<String, Object> getSymStatistics(String controllerHandle,
            String symId) {
        try {
            
            logger.info("getSymStatistics : " + controllerHandle + " symId = "
                    + symId);
            Sym sym = sessionMap.get(symId);
            this.checkForControllerReboot(controllerHandle);
            if (sym == null) {
                return this.createErrorMap(SESSION_NOT_FOUND,
                        "Specified sym was not found " + symId);
            }

            Map<String, Object> stats = sym.getStats();

            Map<String, Object> retval = this.createSuccessMap();
            retval.putAll(stats);
            return retval;
        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }finally {
            release(controllerHandle);
        }
    }

    public Map<String, Object> getBridgeStatistics(String controllerHandle,
            String bridgeId) {
        try {
            this.checkForControllerReboot(controllerHandle);

            logger.info("getBridgeStatistics : " + controllerHandle
                    + " bridgeId " + bridgeId);

            Bridge bridge = bridgeMap.get(bridgeId);
            if (bridge == null) {
                return this.createErrorMap(SESSION_NOT_FOUND,
                        "Specified bridge was not found " + bridgeId);
            }
            
            GetBridgeStatisticsWorkItem workItem = new GetBridgeStatisticsWorkItem(bridgeId);
            DataShuffler.addWorkItem(workItem);
            boolean acquired = workItem.workSem.tryAcquire(TIMEOUT, TimeUnit.MILLISECONDS);
            
            if ( ! acquired ) {
                logger.error("Timed out acquiring workItem sem ");
                return createErrorMap(PROCESSING_ERROR,"SipXrelaySemaphore timed out");
            } else {
                logger.debug("Acquired workItem sem");
            }
            if ( workItem.error ) {
                return createErrorMap(workItem.errorCode,workItem.reason);
            } else {
                return workItem.retval;
            }
            
        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }finally {
            release(controllerHandle);
        }
    }

    public Map<String, Object> signOut(String controllerHandle) {
        try {
            this.checkForControllerReboot(controllerHandle);
            logger.info("signOut " + controllerHandle);
            HashSet<Bridge> rtpBridges = bridgeResourceMap
                    .get(controllerHandle);
            if (rtpBridges != null) {
                for (Bridge rtpBridge : rtpBridges) {
                    rtpBridge.stop();
                }
            }
            bridgeResourceMap.remove(controllerHandle);

            HashSet<Sym> rtpSessions = sessionResourceMap.get(controllerHandle);
            if (rtpSessions != null) {
                for (Sym rtpSession : rtpSessions) {
                  rtpSession.close();      
                }
            }
            sessionResourceMap.remove(controllerHandle);

            return this.createSuccessMap();
        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }finally {
            release(controllerHandle);
        }
    }

    public Map<String, Object> destroySym(String controllerHandle, String symId) {

        try {
            logger.info("destroySym: " + controllerHandle + " symId = "
                    + symId);
            this.checkForControllerReboot(controllerHandle);

            if (sessionMap.containsKey(symId)) {
                Sym sym = sessionMap.get(symId);
                HashSet<Sym> syms = sessionResourceMap.get(controllerHandle);
                syms.remove(sym);
                if (syms.isEmpty()) {
                    sessionResourceMap.remove(controllerHandle);
                }
                sym.close();
                Map<String, Object> retval = this.createSuccessMap();
                return retval;
            } else {

                return this.createErrorMap(SESSION_NOT_FOUND,
                        "Sym with the given Id was not found");
            }

        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }finally {
            release(controllerHandle);
        }
    }

    public Map<String, Object> ping(String controllerHandle) {
        try {
            logger.debug("ping : " + controllerHandle);
            this.checkForControllerReboot(controllerHandle);
            Map<String, Object> retval = createSuccessMap();
            if (sessionResourceMap.get(controllerHandle) != null) {
                HashSet<String> timedOutSyms = new HashSet<String>();

                for (Sym sym : sessionResourceMap.get(controllerHandle)) {
                    if (sym.isTimedOut()) {
                        timedOutSyms.add(sym.getId());
                    }
                }

                if (timedOutSyms.size() > 0) {
                    String[] symArray = new String[timedOutSyms.size()];
                    timedOutSyms.toArray(symArray);
                    retval.put(SYM_SESSION, symArray);
                }

                int nbridges = this.bridgeMap.size();
                retval.put(NBRIDGES, new Integer(nbridges).toString());
            }
            return retval;
        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }finally {
            release(controllerHandle);
        }

    }

    public Map<String, Object> pingAndTest(String controllerHandle,
            String host, int port) {
        try {
            logger.debug("pingAndTest : " + controllerHandle);
            this.checkForControllerReboot(controllerHandle);
            Map<String, Object> retval = createSuccessMap();
            if ( crlfReceiver == null ) {
                restartCrLfReceiver();
            }
            crlfReceiver.sendPing(host,port);
            for (int i = 0; i < 10; i++) {
                if (crlfReceiver.packetRecieved) {
                    break;
                } else {
                    Thread.sleep(5);
                }
            }
            retval.put(PROXY_LIVENESS, new Boolean(
                    this.crlfReceiver.packetRecieved).toString());
            return retval;
        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }finally {
            release(controllerHandle);
        }
    }

    public Map<String, Object> setTimeout(String controllerHandle,
            String symId, int inactivityTimeout) {
        try {
            logger.info("setTimeOut " + controllerHandle + " symId " + symId +  " inactivityTimeout " + inactivityTimeout);
            this.checkForControllerReboot(controllerHandle);

            if (sessionMap.containsKey(symId)) {
                Sym sym = sessionMap.get(symId);
                sym.setInactivityTimeout(inactivityTimeout);
                return this.createSuccessMap();
            } else {
                return this.createErrorMap(SESSION_NOT_FOUND,
                        "Requested SYM Session was not found");
            }
        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }finally {
            release(controllerHandle);
        }
    }

    public Map<String, Object> destroyBridge(String controllerHandle,
            String bridgeId) {
        try {
            logger.info("destroyBridge: " + controllerHandle + " bridgeId "
                    + bridgeId);
            this.checkForControllerReboot(controllerHandle);
            Bridge bridge = bridgeMap.get(bridgeId);
            if (bridge != null) {
                bridgeMap.remove(bridgeId);
                HashSet<Bridge> bridges = bridgeResourceMap
                .get(controllerHandle);
                bridges.remove(bridge);
                if (bridges.isEmpty()) {
                    bridgeResourceMap.remove(controllerHandle);
                }
                for (Sym sym : bridge.getSyms()) {
                    sym.close();
                    sessionMap.remove(sym.getId());
                    HashSet<Sym> syms = sessionResourceMap
                    .get(controllerHandle);
                    if ( syms != null ) {
                        syms.remove(sym);
                        if (syms.isEmpty()) {
                            sessionResourceMap.remove(controllerHandle);
                        }
                    }

                }
                return this.createSuccessMap();
            } else {
                if (logger.isDebugEnabled()) {
                    logger.error("Bridge with the given ID was not found "
                            + bridgeId);
                    return this.createErrorMap(SESSION_NOT_FOUND,
                    "Bridge with the given ID was not found");
                } else {
                    return this.createSuccessMap();
                }
            }
        } catch (Exception ex) {
            logger.error("destroyBridge: " + controllerHandle + " bridgeId "
                    + bridgeId);
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }finally {
            release(controllerHandle);
        }
    }

    /*
     * (non-Javadoc)
     * 
     * @see org.sipfoundry.sipxbridge.symmitron.Symmitron#getReceiverState(java.lang
     *      .String, java.lang.String)
     */
    public Map<String, Object> getReceiverState(String controllerHandle,
            String symId) {
        try {
            logger.info("getReceiverState : " + controllerHandle + " symId "
                    + symId);
            this.checkForControllerReboot(controllerHandle);

            if (sessionMap.containsKey(symId)) {
                Sym sym = sessionMap.get(symId);
                String state = sym.getReceiver().getDatagramChannelState();
                Map retval = this.createSuccessMap();
                retval.put(Symmitron.RECEIVER_STATE, state);
                return retval;
            } else {
                return this.createErrorMap(SESSION_NOT_FOUND,
                "Requested SYM Session was not found");
            }
        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }finally {
            release(controllerHandle);
        }
    }

    /**
     * Test method - stop the xml rpc server.
     */
    public static void stopXmlRpcServer() {
        try {
            SymmitronServer.webServer.stop();
            isWebServerRunning = false;
        } catch (InterruptedException e) {
            logger.error("request processing interrupt", e);
        }
    }

    public static void configtest() throws Exception {
        boolean stunServerAddressOk = false;
        String configDir = System.getProperty("conf.dir", "/etc/sipxpbx");
        String configurationFile = configDir + "/nattraversalrules.xml";
        if (!new File(configurationFile).exists()) {
            System.exit(-1);
        }
        SymmitronConfig config = new SymmitronConfigParser().parse("file:"
                + configurationFile);
        SymmitronServer.setSymmitronConfig(config);

        if (config.getLogFileDirectory() == null) {
            String installRoot = configDir.substring(0, configDir
                    .indexOf("/etc/sipxpbx"));
            config.setLogFileDirectory(installRoot + "/var/log/sipxpbx");
        }
        config.setLogFileName("sipxrelay.log");

        if (config.getLocalAddress() == null) {
            System.err.println("Local address not specified");
            System.exit(-1);
        }

        if (config.getPublicAddress() == null
                && config.getStunServerAddress() == null
                && config.isBehindNat()) {
            System.err
                    .println("Must specify either public address or stun server address");
            System.exit(-1);
        }

        if (config.getPublicAddress() != null) {
            findIpAddress(config.getPublicAddress());
        }

        /*
         * Test if stun server name can be resolved.
         */
        if (config.getStunServerAddress() != null) {
            try {
                findIpAddress(config.getStunServerAddress());
                stunServerAddressOk = true;
            } catch (UnknownHostException ex) {
                System.err
                        .println("WARNING : Cannot resolve STUN server address. Please check configuration.");
            }
        }

        /*
         * Test if local address can be resolved.
         */
        if (config.getLocalAddress() != null) {
            findIpAddress(config.getLocalAddress());
        } else {
            System.err.println("Missing local address. Cannot start sipxrelay");
            System.exit(-1);
        }
        /*
         * Test if port range lower bound is even.
         */
        if (config.getPortRangeLowerBound() % 2 != 0) {
            System.err.println("Please specify even port range lower bound");
            System.exit(-1);
        }

        if (config.getPortRangeUpperBound() <= config.getPortRangeLowerBound()) {
            System.err.println("Please specify valid port range");
            System.exit(-1);
        }

        if ((config.getPortRangeUpperBound() - config.getPortRangeLowerBound()) % 4 != 0) {
            System.err
                    .println("Port range upper bound - Port range lower bound should be a multiple of 4");
            System.exit(-1);
        }

        /*
         * Check if the stun server is running.
         */
        if (stunServerAddressOk && config.getPublicAddress() == null) {
            discoverAddress();
            if (SymmitronServer.getPublicInetAddress() == null) {
                System.err
                        .println("WARNING: STUN Server did not return any meaningful information -- passing configtest with warning");
            }
        }

        System.exit(0);
    }

    private static void initHttpsClient() {
        try {
            // Create empty HostnameVerifier
            HostnameVerifier hv = new HostnameVerifier() {
                public boolean verify(String arg0, SSLSession arg1) {
                    return true;
                }
            };

            HttpsURLConnection.setDefaultHostnameVerifier(hv);

        } catch (Exception ex) {
            logger.fatal("Unexpected exception initializing HTTPS client", ex);
            throw new SymmitronException(ex);
        }
    }

    /**
     * Try an address discovery. If it did not work, then send an alarm. This
     * deals with accidental mis-configurations of the STUN server address.
     */
    private static void tryDiscoverAddress() {
        try {
            try {
                SymmitronServer.findIpAddress(SymmitronServer.symmitronConfig
                        .getStunServerAddress());
                addressResolutionAlarmSent = false;
            } catch (UnknownHostException ex) {
                /*
                 * Cannot resolve address or bad address entered. Carry on
                 * bravely - maybe we will recover.
                 */
                try {
                    if (!addressResolutionAlarmSent) {
                        SymmitronServer.alarmClient.raiseAlarm(
                                SymmitronServer.STUN_ADDRESS_ERROR_ALARM_ID,
                                SymmitronServer.symmitronConfig
                                        .getStunServerAddress());
                        addressResolutionAlarmSent = true;
                        return;
                    }
                } catch (XmlRpcException e) {
                    logger.error("Problem sending Alarm", ex);
                    return;
                }
            }
            /*
             * Address of STUN server successfully resolved. Lets try to contact
             * it
             */

            discoverAddress();
            if (SymmitronServer.getPublicInetAddress() == null && !alarmSent) {
                SymmitronServer.alarmClient.raiseAlarm(STUN_FAILURE_ALARM_ID,
                        SymmitronServer.symmitronConfig.getStunServerAddress());
                alarmSent = true;

            } else if (SymmitronServer.getPublicInetAddress() != null
                    && alarmSent) {
                SymmitronServer.alarmClient.raiseAlarm(STUN_RECOVERY_ALARM_ID,
                        SymmitronServer.symmitronConfig.getStunServerAddress());
                alarmSent = false;
            }
        } catch (Exception ex) {
            try {
                if (!alarmSent) {
                    SymmitronServer.alarmClient.raiseAlarm(
                            STUN_FAILURE_ALARM_ID,
                            SymmitronServer.symmitronConfig
                                    .getStunServerAddress());
                    alarmSent = true;
                }
            } catch (XmlRpcException e) {
                logger.error("Problem sending Alarm", ex);
            }
            logger.error("Error discovering address - stun server down?");
        }
    }

    public static void start() throws Exception {
        SymmitronServer.configDir = System.getProperty("conf.dir",
                "/etc/sipxpbx");
        String configurationFile = configDir + "/nattraversalrules.xml";
        if (!new File(configurationFile).exists()) {
            System.err.println("Configuration file " + configurationFile
                    + " missing.");
            System.exit(-1);
        }

        SymmitronConfig config = new SymmitronConfigParser().parse("file:"
                + configurationFile);

        SymmitronServer.alarmClient = new SipXAlarmClient(config
                .getSipXSupervisorHost(), config.getSipXSupervisorXmlRpcPort());

        InetAddress localAddr = findIpAddress(config.getLocalAddress());

        if (config.getLogFileDirectory() == null) {
            String installRoot = configDir.substring(0, configDir
                    .indexOf("/etc/sipxpbx"));
            config.setLogFileDirectory(installRoot + "/var/log/sipxpbx");
        }
        config.setLogFileName("sipxrelay.log");

        // Configure log4j
        Properties props = new Properties();
        props.setProperty("log4j.rootLogger", "warn, file");
        props.setProperty("log4j.logger.org.sipfoundry.sipxrelay",
                SipFoundryLayout.mapSipFoundry2log4j(config.getLogLevel()).toString());
        props.setProperty("log4j.appender.file", SipFoundryAppender.class.getName());
        props.setProperty("log4j.appender.file.File", config.getLogFileDirectory() + "/" + config.getLogFileName());
        props.setProperty("log4j.appender.file.layout", SipFoundryLayout.class.getName());
        props.setProperty("log4j.appender.file.layout.facility", "JAVA");
        String log4jProps = configDir + "/log4j.properties";
        if (new File(log4jProps).exists()) {
            Properties fileProps = new Properties();
            fileProps.load(new FileInputStream(log4jProps));
            String level = fileProps
                    .getProperty("log4j.logger.org.sipfoundry.sipxrelay");
            if (level != null) {
                props.setProperty("log4j.logger.org.sipfoundry.sipxrelay",level);
            }
        }
      
        PropertyConfigurator.configure(props);

        SymmitronServer.setSymmitronConfig(config);

        logger.info("Checking port range " + config.getPortRangeLowerBound()
                + ":" + config.getPortRangeUpperBound());
        for (int i = config.getPortRangeLowerBound(); i < config
                .getPortRangeUpperBound(); i++) {
            try {
                DatagramSocket sock = new DatagramSocket(i, localAddr);
                sock.close();
            } catch (Exception ex) {
                logger.error(String.format("Failed to bind to %s:%d",
                        localAddr, i), ex);
                throw ex;
            }
        }
        logger.info("Port range checked ");

        if (config.getPublicAddress() == null
                && config.getStunServerAddress() != null) {

            tryDiscoverAddress();
            timer.schedule(new TimerTask() {

                @Override
                public void run() {
                    tryDiscoverAddress();

                }

            }, config.getRediscoveryTime() * 1000,
                    config.getRediscoveryTime() * 1000);
        }

        logger.info("Public address is " + config.getPublicAddress());

        SymmitronServer.initHttpsClient();

        SymmitronServer.startWebServer();
        status = "RUNNING";

    }
    
    public static void printBridges() {
      for ( Bridge bridge : SymmitronServer.bridgeMap.values() ) {
          logger.error("Bridge = " + bridge);
      }      
    }
    
    public static Collection<Sym> getSyms() {
        return SymmitronServer.sessionMap.values();
    }
    
    public static Collection<Bridge> getBridges() {
       return SymmitronServer.bridgeMap.values();
    }

   
    
    public static void main(String[] args) throws Exception {
        try {

            SymmitronServer.startTime = System.currentTimeMillis();
            String command = System.getProperty("sipxrelay.command", "start");
            String filterStr = System.getProperty("sipxrelay.filterStrayPackets","true");
            SymmitronServer.filterStrayPackets = filterStr.equalsIgnoreCase("true");
            logger.info("Stray packet filter is set to " + 
                    SymmitronServer.filterStrayPackets);

            if (command.equals("configtest")) {
                configtest();
            } else if (command.equals("start")) {
                start();
            } else {
                System.err.println("unknown start option " + command);
            }
        
           dataShuffler = new DataShuffler();
           dataShufflerThread = new Thread(dataShuffler);
           dataShufflerThread.start();
          

        } catch (Throwable th) {
            System.err.println("Exiting main: Cause :  " + th.getMessage());
            th.printStackTrace(System.err);
            System.exit(-1);
        }
    }

   

    

}
