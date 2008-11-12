/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.SipStackExt;
import gov.nist.javax.sip.clientauthutils.AuthenticationHelper;

import java.io.File;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Timer;
import java.util.TimerTask;
import java.util.logging.ConsoleHandler;
import java.util.logging.Level;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.ListeningPoint;
import javax.sip.SipProvider;
import javax.sip.address.Address;
import javax.sip.address.SipURI;
import javax.sip.message.Request;

import net.java.stun4j.StunAddress;
import net.java.stun4j.client.NetworkConfigurationDiscoveryProcess;
import net.java.stun4j.client.StunDiscoveryReport;

import org.apache.log4j.Logger;
import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServer;
import org.apache.xmlrpc.server.XmlRpcServerConfigImpl;
import org.apache.xmlrpc.webserver.WebServer;
import org.sipfoundry.sipxbridge.symmitron.SymmitronClient;
import org.sipfoundry.sipxbridge.symmitron.SymmitronConfig;
import org.sipfoundry.sipxbridge.symmitron.SymmitronConfigParser;
import org.sipfoundry.sipxbridge.symmitron.SymmitronException;
import org.sipfoundry.sipxbridge.xmlrpc.SipXbridgeXmlRpcServer;
import org.xbill.DNS.Lookup;
import org.xbill.DNS.Record;
import org.xbill.DNS.SRVRecord;
import org.xbill.DNS.Type;

/**
 * The main class
 * 
 * @author M. Ranganathan
 * 
 */
public class Gateway {

    private static Logger logger = Logger.getLogger(Gateway.class);

    private static String configurationFile = "file:///etc/sipxpbx/sipxbridge.xml";

    static final String SIPXBRIDGE_USER = "sipxbridge";
    /*
     * The account manager -- tracks user accounts. This is populated by reading the
     * sipxbridge.xml configuration file.
     */
    private static AccountManagerImpl accountManager;

    /*
     * The security manager - handles wrapping credentials etc.
     */
    private static AuthenticationHelper authenticationHelper;

    /*
     * The registration manager.
     */
    private static RegistrationManager registrationManager;

    /*
     * Internal SIp Provider
     */
    private static SipProvider internalProvider;

    /*
     * External provider.
     */
    private static SipProvider externalProvider;

    /*
     * External
     */
    private static SipProvider externalTlsProvider;

    /*
     * Back to back user agent manager.
     */
    private static CallControlManager callControlManager;

    /*
     * This is a placeholder - to be replaced by STUN
     */
    private static String globalAddress;

    /*
     * The stun port
     */
    private static final int STUN_PORT = 3478;

    /*
     * A table of timers for re-registration
     */
    private static Timer timer = new Timer();

    /*
     * The Music on hold URL
     */
    private static Address musicOnHoldAddress;

    /*
     * The From address of the gateway.
     */
    private static Address gatewayFromAddress;

    /*
     * THe Webserver for the xml rpc interface.
     */
    private static WebServer webServer;

    /*
     * The Gateway state.
     */
    static GatewayState state = GatewayState.STOPPED;

    /*
     * The time for REGISTER requests.
     */
    private static final int MAX_REGISTRATION_TIMER = 10000;

    /*
     * The rediscovery timer for determining the sipx proxy addresses. Every 30 seconds, look up
     * the proxy addresses.
     */
    private static final int REDISCOVERY_TIMER = 30 * 1000;

    /*
     * Default transport to talk to ITSP
     */
    protected static final String DEFAULT_ITSP_TRANSPORT = "udp";
    
    
    /*
     * Session expires interval.
     */
    protected static int sessionExpires = 30 * 60; 

    /*
     * set to true to enable tls (untested).
     */

    private static boolean isTlsSupportEnabled = false;

    private static int callCount;

    private static boolean isWebServerRunning;

    private static Hashtable<String, SymmitronClient> symmitronClients = new Hashtable<String, SymmitronClient>();

    private static String configurationPath;

    private static HashSet<String> proxyAddressTable = new HashSet<String>();

    private static HashSet<Integer> parkServerCodecs = new HashSet<Integer>();

    private Gateway() {

    }

    static void setConfigurationFileName(String configFileName) {
        Gateway.configurationFile = configFileName;
    }

    static void parseConfigurationFile() {
        ConfigurationParser parser = new ConfigurationParser();
        accountManager = parser.createAccountManager(configurationFile);

    }

    static SymmitronClient initializeSymmitron(String address) {
        SymmitronClient symmitronClient = symmitronClients.get(address);
        if (symmitronClients != null) {
            int symmitronPort;
            if (Gateway.getBridgeConfiguration().getSymmitronXmlRpcPort() != 0) {
                symmitronPort = Gateway.getBridgeConfiguration().getSymmitronXmlRpcPort();
            } else {
                SymmitronConfig symconfig = new SymmitronConfigParser()
                        .parse(Gateway.configurationPath + "/nattraversalrules.xml");
                symmitronPort = symconfig.getXmlRpcPort();
            }
            symmitronClient = new SymmitronClient(address, symmitronPort, callControlManager);
        }
        symmitronClients.put(address, symmitronClient);
        return symmitronClient;
    }

    static void stopXmlRpcServer() {
        try {
            if (webServer != null) {
                webServer.shutdown();
            }
            isWebServerRunning = false;
        } catch (Exception ex) {
            logger.error("Error stopping xml rpc server.", ex);
        }
    }

    public static void startXmlRpcServer() throws GatewayConfigurationException {
        BridgeConfiguration bridgeConfig = accountManager.getBridgeConfiguration();

        if (!isWebServerRunning && bridgeConfig.getXmlRpcPort() != 0) {

            isWebServerRunning = true;

            logger.debug("Starting xml rpc server on port " + bridgeConfig.getXmlRpcPort());
            try {
                webServer = new WebServer(bridgeConfig.getXmlRpcPort(), InetAddress
                        .getByName(bridgeConfig.getLocalAddress()));
            } catch (UnknownHostException e) {
                logger.error("Error creating web server", e);
                throw new GatewayConfigurationException("Error creating web server", e);
            }

            PropertyHandlerMapping handlerMapping = new PropertyHandlerMapping();

            try {
                handlerMapping.addHandler(SipXbridgeXmlRpcServer.SERVER,
                        SipXbridgeXmlRpcServerImpl.class);
            } catch (XmlRpcException e) {
                logger.error("Error configuring RPC Server", e);
                throw new GatewayConfigurationException("Error configuring RPC Server", e);
            }

            XmlRpcServer server = webServer.getXmlRpcServer();

            XmlRpcServerConfigImpl serverConfig = new XmlRpcServerConfigImpl();
            serverConfig.setKeepAliveEnabled(true);

            server.setConfig(serverConfig);
            server.setHandlerMapping(handlerMapping);
            try {
                webServer.start();
            } catch (IOException e) {
                logger.error("Error configuring web server", e);
                throw new GatewayConfigurationException("Error configuring web server", e);
            }
        }
    }

    /**
     * Discover our address using stun.
     * 
     * @throws GatewayConfigurationException
     */
    static void discoverAddress() throws GatewayConfigurationException {
        try {

            BridgeConfiguration bridgeConfiguration = accountManager.getBridgeConfiguration();
            String stunServerAddress = bridgeConfiguration.getStunServerAddress();

            if (stunServerAddress != null) {

                // Todo -- deal with the situation when this port may be taken.
                StunAddress localStunAddress = new StunAddress(Gateway.getLocalAddress(),
                        STUN_PORT);

                java.util.logging.Logger log = java.util.logging.Logger
                        .getLogger("net.java.stun4j");

                Level level = Level.OFF;

                /*
                 * String logLevel = bridgeConfiguration.getLogLevel(); if
                 * (logLevel.equals("INFO")) level = Level.INFO; else if
                 * (logLevel.equals("DEBUG")) level = Level.FINE; else if
                 * (logLevel.equals("WARN")) level = Level.WARNING;
                 */

                log.setLevel(level);
                java.util.logging.FileHandler fileHandler = new java.util.logging.FileHandler(
                        Gateway.getLogFile());
                log.removeHandler(new ConsoleHandler());
                log.addHandler(fileHandler);

                StunAddress serverStunAddress = new StunAddress(stunServerAddress, STUN_PORT);

                NetworkConfigurationDiscoveryProcess addressDiscovery = new NetworkConfigurationDiscoveryProcess(
                        localStunAddress, serverStunAddress);

                addressDiscovery.start();
                StunDiscoveryReport report = addressDiscovery.determineAddress();
                globalAddress = report.getPublicAddress().getSocketAddress().getAddress()
                        .getHostAddress();
                logger.debug("Stun report = " + report);
                if (report.getPublicAddress().getPort() != STUN_PORT) {
                    logger.debug("WARNING External port != internal port ");
                }

            }
        } catch (Exception ex) {
            throw new GatewayConfigurationException("Error discovering  address", ex);
        }
    }

    /**
     * Start timer to rediscover our address.
     * 
     */
    static void startRediscoveryTimer() {
        int rediscoveryTime = Gateway.accountManager.getBridgeConfiguration()
                .getGlobalAddressRediscoveryPeriod();
        TimerTask ttask = new TimerTask() {

            @Override
            public void run() {
                try {
                    Gateway.discoverAddress();
                } catch (Exception ex) {
                    logger.error("Error re-discovering  address");
                }

            }

        };
        Gateway.getTimer().schedule(ttask, rediscoveryTime * 1000, rediscoveryTime * 1000);
    }

    /**
     * Get the proxy addresses. This is done once at startup. We cannot accecpt inbound INVITE
     * from other addresses than the ones on this list.
     */

    static void getSipxProxyAddresses() throws GatewayConfigurationException {
        try {

            Record[] records = new Lookup("_sip._" + getSipxProxyTransport() + "."
                    + getSipxProxyDomain(), Type.SRV).run();

            /* repopulate the proxy address table in case of reconfiguration */
            Gateway.proxyAddressTable.clear();
            if (records == null) {
                logger.debug("SRV record lookup returned null");
                String sipxProxyAddress = getSipxProxyDomain();
                Gateway.proxyAddressTable.add(InetAddress.getByName(sipxProxyAddress)
                        .getHostAddress());
            } else {
                for (Record rec : records) {
                    SRVRecord record = (SRVRecord) rec;
                    String resolvedName = record.getTarget().toString();
                    // Sometimes the DNS (on linux) appends a "." to the end of the
                    // record. The library ought to strip this.
                    if (resolvedName.endsWith(".")) {
                        resolvedName = resolvedName.substring(0, resolvedName.lastIndexOf('.'));
                    }
                    String ipAddress = InetAddress.getByName(resolvedName).getHostAddress();

                    Gateway.proxyAddressTable.add(ipAddress);
                }
            }

            logger.debug("proxy address table = " + proxyAddressTable);
        } catch (Exception ex) {
            logger.error("Cannot do address lookup ", ex);
            throw new GatewayConfigurationException("Could not do dns lookup for "
                    + getSipxProxyDomain(), ex);
        }
    }

    static boolean isAddressFromProxy(String address) {

        return proxyAddressTable.contains(address);

    }

    /**
     * Initialize the bridge.
     * 
     */
    static void initializeSipListeningPoints() {
        try {

            BridgeConfiguration bridgeConfiguration = accountManager.getBridgeConfiguration();

            authenticationHelper = ((SipStackExt) ProtocolObjects.sipStack)
                    .getAuthenticationHelper(accountManager, ProtocolObjects.headerFactory);

            int externalPort = bridgeConfiguration.getExternalPort();
            String externalAddress = bridgeConfiguration.getExternalAddress();
            logger.debug("External Address:port = " + externalAddress + ":" + externalPort);
            ListeningPoint externalUdpListeningPoint = ProtocolObjects.sipStack
                    .createListeningPoint(externalAddress, externalPort, "udp");
            ListeningPoint externalTcpListeningPoint = ProtocolObjects.sipStack
                    .createListeningPoint(externalAddress, externalPort, "tcp");
            if (Gateway.isTlsSupportEnabled) {
                ListeningPoint externalTlsListeningPoint = ProtocolObjects.sipStack
                        .createListeningPoint(externalAddress, externalPort + 1, "tls");
                externalTlsProvider = ProtocolObjects.sipStack
                        .createSipProvider(externalTlsListeningPoint);
            }
            externalProvider = ProtocolObjects.sipStack
                    .createSipProvider(externalUdpListeningPoint);
            externalProvider.addListeningPoint(externalTcpListeningPoint);

            int localPort = bridgeConfiguration.getLocalPort();
            String localIpAddress = bridgeConfiguration.getLocalAddress();

            SipURI mohUri = accountManager.getBridgeConfiguration().getMusicOnHoldName() == null ? null
                    : ProtocolObjects.addressFactory.createSipURI(accountManager
                            .getBridgeConfiguration().getMusicOnHoldName(), Gateway
                            .getSipxProxyDomain());
            if (mohUri != null) {
                musicOnHoldAddress = ProtocolObjects.addressFactory.createAddress(mohUri);
            }

            String domain = Gateway.getSipxProxyDomain();
            gatewayFromAddress = ProtocolObjects.addressFactory
                    .createAddress(ProtocolObjects.addressFactory.createSipURI(SIPXBRIDGE_USER,
                            domain));
            logger.debug("Local Address:port " + localIpAddress + ":" + localPort);

            ListeningPoint internalUdpListeningPoint = ProtocolObjects.sipStack
                    .createListeningPoint(localIpAddress, localPort, "udp");

            ListeningPoint internalTcpListeningPoint = ProtocolObjects.sipStack
                    .createListeningPoint(localIpAddress, localPort, "tcp");

            internalProvider = ProtocolObjects.sipStack
                    .createSipProvider(internalUdpListeningPoint);

            internalProvider.addListeningPoint(internalTcpListeningPoint);

            registrationManager = new RegistrationManager(getWanProvider("udp"));

            callControlManager = new CallControlManager();

        } catch (Throwable ex) {
            ex.printStackTrace();
            logger.error("Cannot initialize gateway", ex);
            throw new GatewayConfigurationException("Cannot initialize gateway", ex);
        }

    }

    static AccountManagerImpl getAccountManager() {
        return Gateway.accountManager;

    }

    static BridgeConfiguration getBridgeConfiguration() {
        return accountManager.getBridgeConfiguration();
    }

    static RegistrationManager getRegistrationManager() {
        return registrationManager;
    }

    static AuthenticationHelper getAuthenticationHelper() {
        return authenticationHelper;
    }

    static SipProvider getWanProvider(String transport) {
        if (transport.equalsIgnoreCase("tls"))
            return externalTlsProvider;
        else
            return externalProvider;
    }

    static SipProvider getLanProvider() {
        return internalProvider;
    }

    /**
     * @return the backToBackUserAgentManager
     */
    static CallControlManager getCallControlManager() {
        return callControlManager;
    }

    /**
     * The local address of the gateway.
     * 
     * @return
     */

    static String getLocalAddress() {
        return accountManager.getBridgeConfiguration().getLocalAddress();
    }

    /**
     * The transport to use to talk to sipx proxy. This is registered in the DNS srv.
     * 
     * @return the proxy transport
     */
    static String getSipxProxyTransport() {
        return accountManager.getBridgeConfiguration().getSipxProxyTransport();
    }

    /**
     * The Sipx proxy domain.
     * 
     * @return the sipx proxy domain name.
     */
    static String getSipxProxyDomain() {
        return accountManager.getBridgeConfiguration().getSipxProxyDomain();
    }

    /**
     * 
     * @return the bridge log level.
     */
    static String getLogLevel() {
        return accountManager.getBridgeConfiguration().getLogLevel();
    }

    /**
     * Get the timeout for media.
     * 
     * @return
     */
    static int getMediaKeepaliveMilisec() {
        return Gateway.accountManager.getBridgeConfiguration().getMediaKeepalive();
    }

    /**
     * Get the sip keepalive
     * 
     * @return
     */
    static int getSipKeepaliveSeconds() {
        return Gateway.accountManager.getBridgeConfiguration().getSipKeepalive();
    }

    /**
     * Get the MOH server Request URI.
     */
    static SipURI getMusicOnHoldUri() {
        try {
            return accountManager.getBridgeConfiguration().getMusicOnHoldName() == null ? null
                    : ProtocolObjects.addressFactory.createSipURI(accountManager
                            .getBridgeConfiguration().getMusicOnHoldName(), Gateway
                            .getSipxProxyDomain());
        } catch (Exception ex) {
            logger.error("Unexpected exception creating Music On Hold URI", ex);
            throw new RuntimeException("Unexpected exception", ex);
        }
    }

    /**
     * @return the Music On Hold server Address.
     */
    static Address getMusicOnHoldAddress() {
        return musicOnHoldAddress;
    }

    /**
     * Get the Gateway Address ( used in From Header ) of requests that originate from the
     * Gateway.
     * 
     * @return an address that can be used in the From Header of request that originate from the
     *         Gateway.
     */
    static Address getGatewayFromAddress() {
        return Gateway.gatewayFromAddress;

    }

    static void registerWithItsp() throws GatewayConfigurationException {
        logger.info("------- REGISTERING--------");
        try {
            Gateway.accountManager.lookupItspAccountAddresses();
            Gateway.accountManager.startAuthenticationFailureTimers();

            for (ItspAccountInfo itspAccount : Gateway.accountManager.getItspAccounts()) {

                if (itspAccount.isRegisterOnInitialization()
                        && itspAccount.getState() != AccountState.INVALID) {
                    Gateway.registrationManager.sendRegistrer(itspAccount);
                }

            }

            /*
             * Wait for successful registration.
             */
            for (int i = 0; i < MAX_REGISTRATION_TIMER / 1000; i++) {

                try {
                    Thread.sleep(1000);
                } catch (InterruptedException ex) {
                    throw new RuntimeException("Unexpected exception registering", ex);
                }

                /*
                 * Check all accounts - if they are Authenticated we are done.
                 */
                boolean allAuthenticated = true;
                for (ItspAccountInfo itspAccount : Gateway.accountManager.getItspAccounts()) {
                    if (itspAccount.isRegisterOnInitialization()
                            && itspAccount.getState() == AccountState.AUTHENTICATING) {
                        allAuthenticated = false;
                        break;
                    }
                }
                if (allAuthenticated)
                    break;

            }

            /*
             * For all those who ask for keepalive and don't need registration, kick off their
             * timers.
             */
            for (ItspAccountInfo itspAccount : Gateway.getAccountManager().getItspAccounts()) {
                if (!itspAccount.isRegisterOnInitialization()
                        && itspAccount.getSipKeepaliveMethod().equals("CR-LF")
                        && itspAccount.getState() != AccountState.INVALID) {
                    itspAccount.startCrLfTimerTask();

                }
            }

        } catch (GatewayConfigurationException ex) {
            logger.fatal(ex);
            throw ex;

        } catch (Exception ex) {
            logger.fatal(ex);
            throw new GatewayConfigurationException(ex.getMessage());
        }
    }

    /**
     * Registers all listeners and starts everything.
     * 
     * @throws Exception
     */
    static void startSipListener() throws GatewayConfigurationException {

        try {
            SipListenerImpl listener = new SipListenerImpl();
            getWanProvider("udp").addSipListener(listener);
            if (Gateway.isTlsSupportEnabled)
                getWanProvider("tls").addSipListener(listener);
            getLanProvider().addSipListener(listener);
            ProtocolObjects.start();
        } catch (Exception ex) {
            throw new GatewayConfigurationException(
                    "Could not start gateway -- check configuration", ex);
        }

    }

    static void startAddressDiscovery() {

        if (Gateway.getGlobalAddress() == null
                && Gateway.accountManager.getBridgeConfiguration().getStunServerAddress() == null) {
            throw new GatewayConfigurationException("Gateway address or stun server required. ");
        }

        if (Gateway.getGlobalAddress() == null) {
            discoverAddress();
            startRediscoveryTimer();
        } else {
            Gateway.accountManager.getBridgeConfiguration().setStunServerAddress(null);
        }

    }

    static void start() throws GatewayConfigurationException {
        if (Gateway.getState() != GatewayState.STOPPED) {
            return;
        }
        Gateway.state = GatewayState.INITIALIZING;

        parkServerCodecs.add(RtpPayloadTypes.getPayloadType("PCMU"));
        parkServerCodecs.add(RtpPayloadTypes.getPayloadType("PCMA"));

        /*
         * If specified, try to contact symmitron.
         */
        if (Gateway.getBridgeConfiguration().getSymmitronHost() != null) {
            int i;
            for (i = 0; i < 8; i++) {
                try {
                    Gateway.initializeSymmitron(Gateway.getBridgeConfiguration()
                            .getSymmitronHost());
                } catch (SymmitronException ex) {
                    logger.error("Symmitron not started -- retrying!");
                    try {
                        Thread.sleep(30000);
                        continue;
                    } catch (InterruptedException e) {

                    }

                }
                break;
            }
            if (i == 8) {
                logger.fatal("Coould not contact symmitron - please start symmitron on "
                        + Gateway.getBridgeConfiguration().getSymmitronHost());

            }
        }

        /*
         * Initialize the JAIN-SIP listening points.
         */
        initializeSipListeningPoints();

        /*
         * Lookup the proxy addresses and keep them cached. Note that this is done just once.
         * sipxbridge will need to be restarted if the DNS records are re-configured. We relup
         * upon sipxsupervisor restarting sipxbridge when dns addresses are reconfigured.
         */

        getSipxProxyAddresses();

        /*
         * Start up the STUN address discovery.
         */
        startAddressDiscovery();

        /*
         * Register the sip listener with the provider.
         */
        startSipListener();

        /*
         * Can start sending outbound calls. Cannot yet gake inbound calls.
         */
        Gateway.state = GatewayState.INITIALIZED;

        /*
         * Register with ITSPs. Now we can take inbound calls
         */
        registerWithItsp();

    }

    /**
     * Stop the gateway. Release any port resources associated with ongoing dialogs and tear down
     * ongoing Music oh
     */
    static synchronized void stop() {
        Gateway.state = GatewayState.STOPPING;
        callControlManager.stop();
        logger.debug("Stopping Gateway");
        // Purge the timer.
        getTimer().purge();
        try {
            for (Dialog dialog : ((SipStackExt) ProtocolObjects.sipStack).getDialogs()) {

                DialogApplicationData dat = DialogApplicationData.get(dialog);
                if (dat != null && dat.musicOnHoldDialog != null
                        && dat.musicOnHoldDialog.getState() != DialogState.TERMINATED) {
                    try {
                        SipProvider provider = ((DialogExt) dat.musicOnHoldDialog)
                                .getSipProvider();
                        Request byeRequest = dat.musicOnHoldDialog.createRequest(Request.BYE);
                        ClientTransaction ctx = provider.getNewClientTransaction(byeRequest);
                        dat.musicOnHoldDialog.sendRequest(ctx);
                    } catch (Exception ex) {
                        logger.error("Exception in dialog termination processing", ex);
                    }

                }
                if (dat != null) {
                    BackToBackUserAgent b2bua = dat.getBackToBackUserAgent();
                    if (b2bua != null) {
                        b2bua.removeDialog(dialog);

                    }

                    if (dat.getBackToBackUserAgent() != null
                            && dat.getBackToBackUserAgent().getCreatingDialog() == dialog) {

                        ItspAccountInfo itspAccountInfo = dat.getBackToBackUserAgent()
                                .getItspAccountInfo();

                        Gateway.decrementCallCount();

                        if (itspAccountInfo != null) {
                            itspAccountInfo.decrementCallCount();
                        }
                    }
                }
            }
        } catch (Exception ex) {
            logger.error("Unexepcted exception occured while stopping bridge", ex);

        }
        // Tear down the sip stack.
        ProtocolObjects.stop();
        Gateway.state = GatewayState.STOPPED;
    }

    /**
     * Get the global address of bridge. This is the publicly routable address of the bridge.
     * 
     * @return
     */
    static String getGlobalAddress() {

        return Gateway.accountManager.getBridgeConfiguration().getGlobalAddress() == null ? globalAddress
                : Gateway.accountManager.getBridgeConfiguration().getGlobalAddress();
    }

    /**
     * Get the global port of the bridge. This is the publicly routable port of the bridge.
     * 
     * @return
     */
    static int getGlobalPort() {
        return Gateway.accountManager.getBridgeConfiguration().getGlobalPort() != -1 ? Gateway.accountManager
                .getBridgeConfiguration().getGlobalPort()
                : Gateway.accountManager.getBridgeConfiguration().getExternalPort();
    }

    /**
     * Get the web server port.
     * 
     * @return the web server port.
     */
    static int getXmlRpcWebServerPort() {
        return Gateway.accountManager.getBridgeConfiguration().getXmlRpcPort();
    }

    /**
     * Gets the current bridge status.
     * 
     * @return
     */
    static GatewayState getState() {

        return Gateway.state;

    }

    /**
     * Get the log file name.
     * 
     * @return the log file name
     * 
     */
    static String getLogFile() {
        return Gateway.getAccountManager().getBridgeConfiguration().getLogFileDirectory()
                + "/sipxbridge.log";
    }

    /**
     * Get the default codec name. Returns null if Re-Invite is supported.
     * 
     */
    static String getCodecName() {
        if (Gateway.isReInviteSupported())
            return null;
        else
            return Gateway.getAccountManager().getBridgeConfiguration().getCodecName();
    }

    /**
     * Get the call limit ( number of concurrent calls)
     */
    static int getCallLimit() {
        return Gateway.getAccountManager().getBridgeConfiguration().getMaxCalls();
    }

    /**
     * Get the call count.
     * 
     * @return -- the call count of the gateway ( number of active calls )
     * 
     */
    static int getCallCount() {

        return Gateway.callCount;
    }

    /**
     * Decrement the call count.
     */
    static void decrementCallCount() {
        if (Gateway.callCount > 0)
            Gateway.callCount--;
    }

    static void incrementCallCount() {
        Gateway.callCount++;

    }

    /**
     * Whether or not delayed offer answer model is supported on Re-INVITE
     * 
     * @return true if supported.
     */
    static boolean isReInviteSupported() {

        return accountManager.getBridgeConfiguration().isReInviteSupported();
    }
    
    static int getSessionExpires() {
        return sessionExpires ;
    }

    /**
     * @return the timer
     */
    static Timer getTimer() {
        return timer;
    }

    /**
     * Session keepalive timer.
     * 
     * @return
     */
    static String getSessionTimerMethod() {
        return Request.INVITE;
    }

    /**
     * Returns true if inbound calls go to an auto attendant.
     * 
     * @return
     */
    static boolean isInboundCallsRoutedToAutoAttendant() {
        return accountManager.getBridgeConfiguration().isInboundCallsRoutedToAutoAttendant();
    }

    /**
     * Place where you want to direct calls ( assuming that you have such a place ).
     * 
     * @return - the address of auto attendant.
     */
    static String getAutoAttendantName() {
        return accountManager.getBridgeConfiguration().getAutoAttendantName();
    }

    /**
     * The XML rpc client connection to the symmitron.
     * 
     * @param address - the address ( extracted from the Via header).
     * @return -- the client to talk to the symmitron.
     */
    static SymmitronClient getSymmitronClient(String address) {
        String lookupAddress = address;
        if (Gateway.getBridgeConfiguration().getSymmitronHost() != null) {
            lookupAddress = Gateway.getBridgeConfiguration().getSymmitronHost();
        }

        SymmitronClient symmitronClient = symmitronClients.get(lookupAddress);
        if (symmitronClient == null) {
            symmitronClient = initializeSymmitron(lookupAddress);
        }
        return symmitronClient;
    }

    /**
     * The set of codecs handled by the park server.
     * 
     * @return
     */
    static HashSet<Integer> getParkServerCodecs() {

        return parkServerCodecs;
    }

    /**
     * The main method for the Bridge.
     * 
     * @param args
     */
    public static void main(String[] args) throws Exception {
        try {

            Gateway.isTlsSupportEnabled = System.getProperty("sipxbridge.enableTls", "false")
                    .equals("true");

            Gateway.configurationPath = System.getProperty("conf.dir", "/etc/sipxpbx");
            Gateway.configurationFile = System.getProperty("conf.dir", "/etc/sipxpbx")
                    + "/sipxbridge.xml";
            String command = System.getProperty("sipxbridge.command", "start");

            if (command.equals("start")) {
                // Wait for the configuration file to become available.
                while (!new File(Gateway.configurationFile).exists()) {
                    Thread.sleep(5 * 1000);
                }
                Gateway.parseConfigurationFile();

                Gateway.start();

                startXmlRpcServer();

            } else if (command.equals("configtest")) {
                try {
                    if (!new File(Gateway.configurationFile).exists()) {
                        System.err.println("sipxbridge.xml does not exist - please check configuration.");
                        System.exit(-1);
                    }
                    Gateway.parseConfigurationFile();
                    BridgeConfiguration configuration = Gateway.accountManager
                            .getBridgeConfiguration();
                    
                    if ( configuration.getGlobalAddress() == null
                            && configuration.getStunServerAddress() == null) {
                        logger.error("Configuration error -- no global address or stun server");
                        System.err.println("sipxbridge.xml: Configuration error: no global address specified and no stun server specified.");
                        System.exit(-1);
                    }
                    
                    if (Gateway.accountManager.getBridgeConfiguration().getExternalAddress()
                            .equals(
                                    Gateway.accountManager.getBridgeConfiguration()
                                            .getLocalAddress())
                            && Gateway.accountManager.getBridgeConfiguration().getExternalPort() == Gateway.accountManager
                                    .getBridgeConfiguration().getLocalPort()) {
                        logger
                                .error("Configuration error -- external address == internal address && external port == internal port");
                        System.err.println("sipxbridge.xml: Configuration error: external address == internal address && external port == internal port");
                        
                        System.exit(-1);
                    }

                    System.exit(0);
                } catch (Exception ex) {
                    System.exit(-1);
                }
            } else {
                logger.error("Bad option ");
            }

        } catch (Throwable ex) {
            ex.printStackTrace();
            logger.fatal("Unexpected exception starting", ex);

        }

    }

}
