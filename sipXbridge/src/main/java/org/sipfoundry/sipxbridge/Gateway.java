/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.SipStackExt;
import gov.nist.javax.sip.SipStackImpl;
import gov.nist.javax.sip.clientauthutils.AuthenticationHelper;

import java.io.File;
import java.io.FileInputStream;
import java.util.Collection;
import java.util.HashSet;
import java.util.PriorityQueue;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentHashMap;
import java.util.logging.Handler;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLSession;
import javax.sip.Dialog;
import javax.sip.ListeningPoint;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.address.Address;
import javax.sip.address.Hop;
import javax.sip.address.SipURI;
import javax.sip.message.Request;

import net.java.stun4j.StunAddress;
import net.java.stun4j.client.NetworkConfigurationDiscoveryProcess;
import net.java.stun4j.client.StunDiscoveryReport;

import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.SimpleLayout;
import org.apache.xmlrpc.XmlRpcException;
import org.sipfoundry.commons.alarm.SipXAlarmClient;
import org.sipfoundry.commons.log4j.SipFoundryAppender;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.commons.siprouter.FindSipServer;
import org.sipfoundry.sipxrelay.SymmitronClient;
import org.sipfoundry.sipxbridge.xmlrpc.SipXbridgeXmlRpcClient;

/**
 * The main class
 *
 * @author M. Ranganathan
 *
 */
public class Gateway {

    private static Logger logger = Logger.getLogger(Gateway.class.getPackage().getName());

    private static String configurationFile = "file:///etc/sipxpbx/sipxbridge.xml";

    /*
     * This is a reserved name for the sipxbridge user.
     */
    static final String SIPXBRIDGE_USER = "~~id~bridge";
    /*
     * The account manager -- tracks user accounts. This is populated by reading
     * the sipxbridge.xml configuration file.
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
     * Internal SIp Provider. If TLS is supported for internal
     * connections then the bridge does not accept TCP and udp 
     * packets from the proxy server. 
     */
    private static SipProvider internalProvider;
    
   
    /*
     * External provider (UDP and TCP)
     */
    private static SipProvider externalProvider;

    /*
     * External provider (TLS)
     */
    private static SipProvider externalTlsProvider;

    /*
     * CallControl router.
     */
    private static CallControlManager callControlManager;

    /*
     * The allocator for back to back user agents.
     */

    private static BackToBackUserAgentFactory backToBackUserAgentFactory;

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
     * The Gateway state.
     */
    static GatewayState state = GatewayState.STOPPED;

    /*
     * The time for REGISTER requests.
     */
    private static final int MAX_REGISTRATION_TIMER = 10000;

    /*
     * Default transport to talk to ITSP
     */
    protected static final String DEFAULT_ITSP_TRANSPORT = "udp";

    /*
     * Min value for session timer ( seconds ).
     */
    protected static final int MIN_EXPIRES = 90;

    /*
     * Advance timer by 10 seconds for session timer.
     */
    protected static final int TIMER_ADVANCE = 45;

    /*
     * set to true to enable tls for sip signaling (untested).
     */

    private static boolean isTlsSupportEnabled = true;

    private static ConcurrentHashMap<String, SymmitronClient> symmitronClients = new ConcurrentHashMap<String, SymmitronClient>();

    static String configurationPath;

    static SipFoundryAppender logAppender;

    static NetworkConfigurationDiscoveryProcess addressDiscovery = null;

    private static SipXAlarmClient alarmClient;

    private static SipURI proxyURI;

    private static final String STUN_FAILED_ALARM_ID = "STUN_ADDRESS_DISCOVERY_FAILED";

    private static final String STUN_OK = "STUN_ADDRESS_DISCOVERY_RECOVERED";

    private static final String STUN_PUBLIC_ADDRESS_CHANGED_ALARM_ID = "STUN_PUBLIC_ADDRESS_CHANGED";

    static final String ACCOUNT_NOT_FOUND_ALARM_ID = "SIPX_BRIDGE_ITSP_ACCOUNT_NOT_FOUND";

    static final String SIPXBRIDGE_ACCOUNT_CONFIGURATION_ERROR_ALARM_ID = "SIPX_BRIDGE_ACCOUNT_CONFIGURATION_ERROR";

    static final String SIPX_BRIDGE_OPERATION_TIMED_OUT = "SIPX_BRIDGE_OPERATION_TIMED_OUT";

    static final String SIPX_BRIDGE_ITSP_SERVER_FAILURE = "SIPX_BRIDGE_ITSP_SERVER_FAILURE";

    static final String SIPX_BRIDGE_AUTHENTICATION_FAILED = "SIPX_BRIDGE_AUTHENTICATION_FAILED";

    static final String SIPX_BRIDGE_ITSP_ACCOUNT_CONFIGURATION_ERROR = "SIPX_BRIDGE_ITSP_ACCOUNT_CONFIGURATION_ERROR";

    static final int REGISTER_DELTA = 5;

    public static final String SIPX_BRIDGE_ACCOUNT_OK = "SIPX_BRIDGE_ACCOUNT_OK";

    public static final int DEFAULT_SESSION_TIMER_INTERVAL = 1800;

    private static int oldStunPort = -1;

    private static HashSet<String> supportedTransports = new HashSet<String>(); 

    // ///////////////////////////////////////////////////////////////////////

    /**
     * Make sure nobody calls this constructor.
     */
    private Gateway() {

    }

    private static void setConfigurationPath() {
        Gateway.configurationPath = System.getProperty("conf.dir",
                "/etc/sipxpbx");
    }

    static void setConfigurationFileName(String configFileName) {
        Gateway.configurationFile = configFileName;
    }

    static void parseConfigurationFile() {
        Gateway.setConfigurationPath();
        Gateway.configurationFile = Gateway.configurationPath
                + "/sipxbridge.xml";

        if (!new File(Gateway.configurationFile).exists()) {
            System.err.println(String.format(
                    "Configuration %s file not found -- exiting",
                    Gateway.configurationFile));
            System.exit(-1);
        }
        ConfigurationParser parser = new ConfigurationParser();
        accountManager = parser.createAccountManager(configurationFile);
    }

    static SymmitronClient initializeSymmitron(String address) {
        /*
         * Looks up a symmitron for a given address.
         */
        SymmitronClient symmitronClient = symmitronClients.get(address);
        if (symmitronClient == null) {
            int symmitronPort;
            boolean isSecure = false;

            symmitronPort = Gateway.getBridgeConfiguration()
                    .getSymmitronXmlRpcPort();
            isSecure = Gateway.getBridgeConfiguration().isSecure();

            symmitronClient = new SymmitronClient(address, symmitronPort,
                    isSecure, callControlManager);
            symmitronClients.put(address, symmitronClient);
        }

        return symmitronClient;
    }

    /**
     * Initialize the loggers for the libraries used.
     *
     * @throws SipXbridgeExcception -
     *             if logging initialization failed.
     */
    static void initializeLogging() throws SipXbridgeException {
        try {
           
            BridgeConfiguration bridgeConfiguration = Gateway
                    .getBridgeConfiguration();
            String logLevel = bridgeConfiguration.getLogLevel();

            java.util.logging.Level level = java.util.logging.Level.OFF;
            if (logLevel.equals("INFO"))
                level = java.util.logging.Level.INFO;
            else if (logLevel.equals("DEBUG"))
                level = java.util.logging.Level.FINE;
            else if (logLevel.equals("TRACE"))
                level = java.util.logging.Level.FINER;
            else if (logLevel.equals("WARN"))
                level = java.util.logging.Level.WARNING;

            /*
             * BUGBUG For now turn off Logging on STUN4j. It writes to stdout.
             */
            level = java.util.logging.Level.OFF;

            java.util.logging.Logger log = java.util.logging.Logger
                    .getLogger("net.java.stun4j");
            log.setLevel(level);
            java.util.logging.FileHandler fileHandler = new java.util.logging.FileHandler(
                    Gateway.getLogFile(),true);

            /*
             * Remove all existing handlers.
             */
            for (Handler handler : log.getHandlers()) {
                log.removeHandler(handler);
            }

            /*
             * Add the file handler.
             */
            log.addHandler(fileHandler);

            Gateway.logAppender = new SipFoundryAppender(
                    new SipFoundryLayout(), Gateway.getLogFile(),true);
            
            logger.setLevel(Level.toLevel(logLevel));

            logger.addAppender(logAppender);
        } catch (Exception ex) {
            throw new SipXbridgeException("Error initializing logging", ex);
        }

    }

    /**
     * Discover our address using stun.
     *
     * @throws SipXbridgeException
     */
    static void discoverAddress() throws SipXbridgeException {

        try {

            BridgeConfiguration bridgeConfiguration = accountManager
                    .getBridgeConfiguration();
            String stunServerAddress = bridgeConfiguration
                    .getStunServerAddress();
            String oldPublicAddress = Gateway.getGlobalAddress();

            if (stunServerAddress != null) {
                // Todo -- deal with the situation when this port may be taken.
                if (addressDiscovery == null ) {
                    int localStunPort = STUN_PORT + 2;

                    StunAddress localStunAddress = new StunAddress(Gateway
                            .getLocalAddress(), localStunPort);

                    StunAddress serverStunAddress = new StunAddress(
                            stunServerAddress, STUN_PORT);

                    addressDiscovery = new NetworkConfigurationDiscoveryProcess(
                            localStunAddress, serverStunAddress);

                    addressDiscovery.start();
                }
                StunDiscoveryReport report = addressDiscovery
                        .determineAddress();
                if ( report == null || report.getPublicAddress() == null ) {
                    logger.warn("STUN Error : Global address could not be found");
                    try {
                        if ( addressDiscovery != null ) {
                            addressDiscovery.shutDown();
                        }
                    } catch (Exception e ) {
                        logger.error("Error shutting down address discovery ", e);
                    } finally {
                        addressDiscovery = null;
                    }
                    return;
                }

                globalAddress = report.getPublicAddress().getSocketAddress()
                        .getAddress().getHostAddress();
                logger.debug("Stun report = " + report);

                if ( oldPublicAddress != null && !oldPublicAddress.equals(globalAddress) ||
                    ( oldStunPort != -1 && oldStunPort != report.getPublicAddress().getPort() ) ) {
                    Gateway.getAlarmClient()
                    .raiseAlarm(
                            Gateway.STUN_PUBLIC_ADDRESS_CHANGED_ALARM_ID,
                            globalAddress);
                }
                oldStunPort = report.getPublicAddress().getPort();

                if (report.getPublicAddress().getPort() != STUN_PORT + 2) {
                    logger
                            .warn("WARNING External port != internal port your NAT may not be symmetric.");
                }

            }
        } catch (Exception ex) {
            /*
             * If problem finding address. release the port.
             */
            logger.error("Error discovering  address", ex);
            try {
                if ( addressDiscovery != null ) {
                    addressDiscovery.shutDown();
                }
            } catch (Exception e ) {
                logger.error("Error shutting down address discovery ", e);
            } finally {
                addressDiscovery = null;
            }
            return;
        } finally {
           logger.debug("global address = " + globalAddress);
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
            boolean alarmSent;

            @Override
            public void run() {
                try {
                    Gateway.discoverAddress();
                    if (Gateway.getGlobalAddress() == null && !alarmSent) {
                        Gateway.getAlarmClient()
                                .raiseAlarm(
                                        Gateway.STUN_FAILED_ALARM_ID,
                                        getBridgeConfiguration()
                                                .getStunServerAddress());
                        alarmSent = true;
                    } else if (Gateway.getGlobalAddress() != null && alarmSent) {
                        Gateway.getAlarmClient()
                                .raiseAlarm(
                                        Gateway.STUN_OK,
                                        getBridgeConfiguration()
                                                .getStunServerAddress());
                        alarmSent = false;
                    }
                } catch (Exception ex) {
                    try {
                        if (!alarmSent) {
                            Gateway.getAlarmClient().raiseAlarm(
                                    Gateway.STUN_FAILED_ALARM_ID,
                                    getBridgeConfiguration()
                                            .getStunServerAddress());

                            alarmSent = true;
                        }
                    } catch (XmlRpcException e) {
                        logger.error("Could not send alarm", e);
                    }
                    logger.error("Error re-discovering  address", ex);
                }

            }

        };

        Gateway.discoverAddress();

        Gateway.getTimer().schedule(ttask, rediscoveryTime * 1000,
                rediscoveryTime * 1000);
    }

    /**
     * Get the proxy URI
     */
    static SipURI getProxyURI() {

        return Gateway.proxyURI;
    }

    /**
     * Get the proxy addresses. This is done once at startup. We cannot accecpt
     * inbound INVITE from other addresses than the ones on this list.
     */

    static PriorityQueue<Hop> initializeSipxProxyAddresses()
            throws SipXbridgeException {
        try {

            FindSipServer serverFinder = new FindSipServer(logger);
            SipURI proxyUri = getProxyURI();

            Collection<Hop> hops = serverFinder.findSipServers(proxyUri);
            PriorityQueue<Hop> proxyAddressTable = new PriorityQueue<Hop>();
            proxyAddressTable.addAll(hops);
            logger.debug("proxy address table = " + proxyAddressTable);
            return proxyAddressTable;
        } catch (Exception ex) {
            logger.error("Cannot do address lookup ", ex);
            throw new SipXbridgeException("Could not do dns lookup for "
                    + getSipxProxyDomain(), ex);
        }
    }

    static boolean isAddressFromProxy(String address) {

        PriorityQueue<Hop> proxyAddressTable = initializeSipxProxyAddresses();
        for (Hop hop : proxyAddressTable) {
            if (hop.getHost().equals(address)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Initialize the bridge.
     *
     */
    static void initializeSipListeningPoints() {
        try {

            BridgeConfiguration bridgeConfiguration = accountManager
                    .getBridgeConfiguration();

            authenticationHelper = ((SipStackExt) ProtocolObjects.getSipStack())
                    .getAuthenticationHelper(accountManager,
                            ProtocolObjects.headerFactory);

            int externalPort = bridgeConfiguration.getExternalPort();
            String externalAddress = bridgeConfiguration.getExternalAddress();
            logger.debug("External Address:port = " + externalAddress + ":"
                    + externalPort);
            ListeningPoint externalUdpListeningPoint = ProtocolObjects
                    .getSipStack().createListeningPoint(externalAddress,
                            externalPort, "udp");
            Gateway.supportedTransports.add("udp");
            ListeningPoint externalTcpListeningPoint = ProtocolObjects
                    .getSipStack().createListeningPoint(externalAddress,
                            externalPort, "tcp");
            Gateway.supportedTransports.add("tcp");
            if (Gateway.isTlsSupportEnabled) {
                logger.debug("tlsSupport is enabled -- creating TLS Listening point and provider");
                ListeningPoint externalTlsListeningPoint = ProtocolObjects
                        .getSipStack().createListeningPoint(externalAddress,
                                externalPort + 1, "tls");
                externalTlsProvider = ProtocolObjects.getSipStack()
                        .createSipProvider(externalTlsListeningPoint);
                Gateway.supportedTransports.add("tls");
            }
            externalProvider = ProtocolObjects.getSipStack().createSipProvider(
                    externalUdpListeningPoint);
            externalProvider.addListeningPoint(externalTcpListeningPoint);

            int localPort = bridgeConfiguration.getLocalPort();
            String localIpAddress = bridgeConfiguration.getLocalAddress();

            SipURI mohUri = Gateway.getMusicOnHoldUri();
            if (mohUri != null) {
                musicOnHoldAddress = ProtocolObjects.addressFactory
                        .createAddress(mohUri);
            }

            String domain = Gateway.getSipxProxyDomain();
            gatewayFromAddress = ProtocolObjects.addressFactory
                    .createAddress(ProtocolObjects.addressFactory.createSipURI(
                            SIPXBRIDGE_USER, domain));
            logger.debug("Local Address:port " + localIpAddress + ":"
                    + localPort);

            if ( !Gateway.getSipxProxyTransport().equalsIgnoreCase("tls")) {
                ListeningPoint internalUdpListeningPoint = ProtocolObjects
                .getSipStack().createListeningPoint(localIpAddress,
                        localPort, "udp");

                ListeningPoint internalTcpListeningPoint = ProtocolObjects
                .getSipStack().createListeningPoint(localIpAddress,
                        localPort, "tcp");

                internalProvider = ProtocolObjects.getSipStack().createSipProvider(
                        internalUdpListeningPoint);

                internalProvider.addListeningPoint(internalTcpListeningPoint);
            } else {
                logger.debug("tlsSupport is for proxy enabled -- creating TLS Listening point and provider");
                ListeningPoint internalTlsListeningPoint = ProtocolObjects
                        .getSipStack().createListeningPoint(localIpAddress,
                                localPort, "tls");
                internalProvider = ProtocolObjects.getSipStack()
                        .createSipProvider(internalTlsListeningPoint);
                internalProvider.addListeningPoint(internalTlsListeningPoint);        
            }

            registrationManager = new RegistrationManager();

            callControlManager = new CallControlManager();

            backToBackUserAgentFactory = new BackToBackUserAgentFactory();

        } catch (Throwable ex) {
            ex.printStackTrace();
            logger.error("Cannot initialize gateway", ex);
            throw new SipXbridgeException("Cannot initialize gateway", ex);
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

    static SipXAlarmClient getAlarmClient() {
        if (alarmClient == null) {
            String supervisorHost = getBridgeConfiguration()
                    .getSipXSupervisorHost();
            logger.debug("supervisorHost = " + supervisorHost);
            alarmClient = new SipXAlarmClient(supervisorHost,
                    getBridgeConfiguration().getSipXSupervisorXmlRpcPort());
        }

        return alarmClient;
    }

    static AuthenticationHelper getAuthenticationHelper() {
        return authenticationHelper;
    }

    static SipProvider getWanProvider(String transport) {
        if (transport.equalsIgnoreCase("tls")) {
            return externalTlsProvider;
        } else {
            return externalProvider;
        }
    }

    static SipProvider getLanProvider() {
        return internalProvider;
    }

    /**
     * @return the call control router.
     */
    static CallControlManager getCallControlManager() {
        return callControlManager;
    }

    /**
     * Get the back to back user agent factory.
     *
     * @return the back to back user agent factory.
     */
    static BackToBackUserAgentFactory getBackToBackUserAgentFactory() {
        return backToBackUserAgentFactory;
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
     * The transport to use to talk to sipx proxy. This is registered in the DNS
     * srv.
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

        return Gateway.accountManager.getBridgeConfiguration()
                .getMediaKeepalive();
    }

    /**
     * Get the sip keepalive
     *
     * @return
     */
    static int getSipKeepaliveSeconds() {
        return Gateway.accountManager.getBridgeConfiguration()
                .getSipKeepalive();
    }

    /**
     * Get the MOH server Request URI.
     */
    static SipURI getMusicOnHoldUri() {
        try {
            if (getBridgeConfiguration().getMusicOnHoldName() == null) {
                return null;
            } else if (getBridgeConfiguration().getMusicOnHoldName().indexOf(
                    "@") == -1) {
                return ProtocolObjects.addressFactory.createSipURI(
                        getBridgeConfiguration().getMusicOnHoldName(), Gateway
                                .getSipxProxyDomain());
            } else {
                return (SipURI) ProtocolObjects.addressFactory.createURI("sip:"
                        + getBridgeConfiguration().getMusicOnHoldName());
            }
        } catch (Exception ex) {
            logger.error("Unexpected exception creating Music On Hold URI", ex);
            throw new SipXbridgeException("Unexpected exception", ex);
        }
    }

    /**
     * @return the Music On Hold server Address.
     */
    static Address getMusicOnHoldAddress() {
        return musicOnHoldAddress;
    }

    /**
     * Get the Gateway Address ( used in From Header ) of requests that
     * originate from the Gateway.
     *
     * @return an address that can be used in the From Header of request that
     *         originate from the Gateway.
     */
    static Address getGatewayFromAddress() {
        return Gateway.gatewayFromAddress;

    }

    static void registerWithItsp() throws SipXbridgeException {
        logger.info("------- REGISTERING--------");
        /*
         * Check for mandatory fields.
         */
        boolean invalidAccountDetected = false;
        HashSet<ItspAccountInfo> invalidItspAccounts = new HashSet<ItspAccountInfo>();
        for (ItspAccountInfo accountInfo : Gateway.accountManager
                .getItspAccounts()) {
            if (accountInfo.getProxyDomain() == null) {
                invalidAccountDetected = true;
                invalidItspAccounts.add(accountInfo);
            }

            if (accountInfo.isRegisterOnInitialization() && accountInfo.getUserName() == null) {
                System.err
                        .println("User Name is mandatory for ITSP accounts requiring Registration. Check ITSP account "
                                + accountInfo.getProxyDomain());
                invalidAccountDetected = true;
                invalidItspAccounts.add(accountInfo);
            }
        }
        try {
            if ( invalidAccountDetected ) {
                Gateway.getAlarmClient().raiseAlarm(Gateway.SIPX_BRIDGE_ITSP_ACCOUNT_CONFIGURATION_ERROR);
            }
        } catch ( XmlRpcException ex) {
            logger.error("Problem sending alarm",ex);
        }

        /*
         * Remove all the bad accounts from the collection of accounts.
         */
        for (ItspAccountInfo badAccount : invalidItspAccounts) {
            Gateway.accountManager.getItspAccounts().remove(badAccount);
        }


        try {
            Gateway.accountManager.startAuthenticationFailureTimers();

            for (ItspAccountInfo itspAccount : Gateway.accountManager
                    .getItspAccounts()) {

                if (itspAccount.isRegisterOnInitialization()
                        && itspAccount.getState() != AccountState.INVALID && itspAccount.isEnabled()) {
                    try {
                        Gateway.registrationManager.sendRegistrer(itspAccount,null,1L);
                    } catch (SipException ex) {
                        logger.error("Exception sending REGISTER to "
                                + itspAccount.getProxyDomain());
                        // Maybe an route could not be found so start a timer to
                        // keep trying
                        TimerTask ttask = new RegistrationTimerTask(itspAccount,null,1L);
                        // Retry after 60 seconds.
                        timer.schedule(ttask, 60 * 1000);
                        if (!itspAccount.isAlarmSent()) {
                            Gateway.getAlarmClient()
                                    .raiseAlarm(
                                            Gateway.SIPXBRIDGE_ACCOUNT_CONFIGURATION_ERROR_ALARM_ID,
                                            itspAccount.getProxyDomain());
                            itspAccount.setAlarmSent(true);
                        }
                    }
                }

            }

            /*
             * Wait for successful registration. After this period, all well
             * behaved accounts will have registered.
             */
            for (int i = 0; i < MAX_REGISTRATION_TIMER / 1000; i++) {

                try {
                    Thread.sleep(1000);
                } catch (InterruptedException ex) {
                    throw new SipXbridgeException(
                            "Unexpected exception registering", ex);
                }

                /*
                 * Check all accounts - if they are Authenticated we are done.
                 */
                boolean allAuthenticated = true;
                for (ItspAccountInfo itspAccount : Gateway.accountManager
                        .getItspAccounts()) {
                    if (itspAccount.isRegisterOnInitialization()
                            && itspAccount.getState() == AccountState.AUTHENTICATING) {
                        allAuthenticated = false;
                        break;
                    }
                }
                if (allAuthenticated) {
                    break;
                }

            }

            /*
             * For all those who ask for keepalive and don't need registration,
             * kick off their timers.
             */
            for (ItspAccountInfo itspAccount : Gateway.getAccountManager()
                    .getItspAccounts()) {
                if (!itspAccount.isRegisterOnInitialization()
                        && itspAccount.getSipKeepaliveMethod().equals("CR-LF")
                        && itspAccount.getState() != AccountState.INVALID) {
                    itspAccount.startCrLfTimerTask();

                }
            }

        } catch (SipXbridgeException ex) {
            logger.fatal(ex);
            throw ex;

        } catch (Exception ex) {
            logger.fatal("Could not initialize bridge",ex);
            throw new SipXbridgeException(ex.getMessage());
        }
    }

    /**
     * Registers all listeners and starts everything.
     *
     * @throws Exception
     */
    static void startSipListener() throws SipXbridgeException {

        try {
            SipListenerImpl listener = new SipListenerImpl();
            getWanProvider("udp").addSipListener(listener);
            if (Gateway.isTlsSupportEnabled) {
                getWanProvider("tls").addSipListener(listener);
            }
            getLanProvider().addSipListener(listener);
            ProtocolObjects.start();
        } catch (Exception ex) {
            throw new SipXbridgeException(
                    "Could not start gateway -- check configuration", ex);
        }

    }

    static void startAddressDiscovery() {

        if (Gateway.getGlobalAddress() == null
                && Gateway.accountManager.getBridgeConfiguration()
                        .getStunServerAddress() == null) {
            throw new SipXbridgeException(
                    "Gateway address or stun server required. ");
        }

        if (Gateway.getGlobalAddress() == null) {
            startRediscoveryTimer();
        } else {
            Gateway.accountManager.getBridgeConfiguration()
                    .setStunServerAddress(null);
        }
    }

    /**
     * Configuration test. exit with status code 0 if success and -1 if fail.
     *
     */
    static void configtest() {

        Logger.getLogger("org.sipfoundry.sipxbridge").addAppender(
                new ConsoleAppender(new SimpleLayout()));

        BridgeConfiguration configuration = Gateway.accountManager
                .getBridgeConfiguration();

        if (configuration.getGlobalAddress() == null
                && configuration.getStunServerAddress() == null) {

            System.err
                    .println("sipxbridge.xml: Configuration error: no global address specified and no stun server specified.");
            System.exit(-1);
        }
        if (Gateway.accountManager.getBridgeConfiguration()
                .getExternalAddress() == null) {
            System.err
                    .println("Missing configuration parameter <external-address>");
            System.exit(-1);
        }

        if (Gateway.accountManager.getBridgeConfiguration().getLocalAddress() == null) {
            System.err
                    .println("Missing configuration parameter <local-address>");
            System.exit(-1);
        }

        if (Gateway.accountManager.getBridgeConfiguration()
                .getExternalAddress().equals(
                        Gateway.accountManager.getBridgeConfiguration()
                                .getLocalAddress())
                && Gateway.accountManager.getBridgeConfiguration()
                        .getExternalPort() == Gateway.accountManager
                        .getBridgeConfiguration().getLocalPort()) {
            System.err
                    .println("Configuration error: external address == internal address && external port == internal port");

            System.exit(-1);
        }



        if (Gateway.accountManager.getBridgeConfiguration()
                .getStunServerAddress() != null
                && Gateway.accountManager.getBridgeConfiguration()
                        .getGlobalAddress() == null) {
            Gateway.discoverAddress();
            if (Gateway.getGlobalAddress() == null) {
                System.err
                        .println("WARNING: Could not discover public address. Check your STUN server settings or specify public address");
            }

        }
        System.exit(0);
    }

    /**
     * Start the gateway.
     *
     * @throws SipXbridgeException --
     *             if Gateway start failed.
     *
     */
    static void start() throws SipXbridgeException {
        // Wait for the configuration file to become available.

        Gateway.initializeLogging();

        if (Gateway.getState() != GatewayState.STOPPED) {
            logger
                    .debug("Gateway State is " + Gateway.getState()
                            + " Aborting");
            return;
        }

        Gateway.state = GatewayState.INITIALIZING;

        try {

            Gateway.proxyURI = ProtocolObjects.addressFactory.createSipURI(
                    null, getBridgeConfiguration().getSipxProxyDomain());
        } catch (Exception ex) {
            logger.error("Error initializing proxy address", ex);
            throw new SipXbridgeException(ex);
        }

        /*
         * Start the web server first so that you can restart reliably.
         */
        SipXbridgeXmlRpcServerImpl.startXmlRpcServer();

        /*
         * Initialize the JAIN-SIP listening points.
         */
        initializeSipListeningPoints();

        /* Test to see that we can do an address lookup */

        initializeSipxProxyAddresses();

        /*
         * Start up the STUN address discovery.
         */
        startAddressDiscovery();

        /*
         * Register the sip listener with the provider.
         */
        startSipListener();

        /*
         * Register with ITSPs. Now we can take inbound calls
         */
        while (Gateway.getGlobalAddress() == null) {
            try {
                Thread.sleep(5000);
            } catch (InterruptedException ex) {
                logger.error("Sleep interrupted.");
                System.exit(0);
            }
        }
        logger.debug("Global address = " + Gateway.getGlobalAddress());

        /*
         * Can start sending outbound calls. Cannot yet gake inbound calls.
         */
        Gateway.state = GatewayState.INITIALIZED;

        /*
         * Found the global address. Now can REGISTER.
         */

        registerWithItsp();

        /*
         * Initialize connection with sipxrelay.
         */
        if (getBridgeConfiguration().isSecure()) {
            Gateway.initHttpsClient();
        }

    }

    /**
     * Stop the gateway. Release any port resources associated with ongoing
     * dialogs and tear down ongoing Music on hold. This is called when stopping
     * the gateway but not exiting the Gateway process.
     */
    static synchronized void stop() {
        Gateway.state = GatewayState.STOPPING;

        logger.debug("Stopping Gateway");
        // Purge the timer.
        getTimer().purge();
        try {
            /*
             * De-register from all ITSP accounts.
             */
            for (ItspAccountInfo itspAccount : Gateway.getAccountManager()
                    .getItspAccounts()) {
                if (itspAccount.isRegisterOnInitialization()) {
                    registrationManager.sendDeregister(itspAccount);
                }
            }

            /*
             * Tear down all ongoing calls.
             */
            Collection<Dialog> dialogs = ((SipStackImpl) ProtocolObjects
                    .getSipStack()).getDialogs();
            for (Dialog dialog : dialogs) {
                if (dialog.getApplicationData() instanceof DialogContext) {
                    BackToBackUserAgent b2bua = DialogContext
                            .getBackToBackUserAgent(dialog);
                    b2bua.tearDown(Gateway.SIPXBRIDGE_USER,
                            ReasonCode.BRIDGE_STOPPING, "Bridge Stopping");
                }
            }

            for (SymmitronClient client : Gateway.symmitronClients.values()) {
                client.signOut();
            }

        } catch (Exception ex) {
            logger.error("Unexepcted exception occured while stopping bridge",
                    ex);

        }
        Gateway.state = GatewayState.STOPPED;
        // Tear down the sip stack.
        ProtocolObjects.stop();

    }

    /**
     * Exit the gateway. Caution! This is called to actually stop the process
     * and exit.
     */
    private static void exit() {
        /*
         * Stop bridge, release all resources and exit.
         */
        Gateway.initializeLogging();
        logger.debug("exit()");
        /*
         * Initialize the HTTPS client.
         */
        if (Gateway.getBridgeConfiguration().isSecure()) {
            Gateway.initHttpsClient();
        }

        /*
         * Connect to the sipxbridge server and ask him to exit.
         */
        SipXbridgeXmlRpcClient client = new SipXbridgeXmlRpcClient(Gateway
                .getBridgeConfiguration().getExternalAddress(), Gateway
                .getBridgeConfiguration().getXmlRpcPort(), Gateway
                .getBridgeConfiguration().isSecure());
        client.stop();
        System.exit(0);
    }

    /**
     * Get the global address of bridge. This is the publicly routable address
     * of the bridge.
     *
     * @return
     */
    static String getGlobalAddress() {

        return Gateway.accountManager.getBridgeConfiguration()
                .getGlobalAddress() == null ? globalAddress
                : Gateway.accountManager.getBridgeConfiguration()
                        .getGlobalAddress();
    }

    /**
     * Get the global port of the bridge. This is the publicly routable port of
     * the bridge.
     *
     * @return
     */
    static int getGlobalPort(String transport) {
        
        int port =  Gateway.accountManager.getBridgeConfiguration().getGlobalPort() != -1 ? Gateway.accountManager
                .getBridgeConfiguration().getGlobalPort()
                : Gateway.accountManager.getBridgeConfiguration()
                        .getExternalPort();
                
        if (transport.equalsIgnoreCase("tls")) {
            return port +1;
        } else {
            return port;
        }
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
        return Gateway.getAccountManager().getBridgeConfiguration()
                .getLogFileDirectory()
                + "/sipxbridge.log";
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
     * Get the Music On hold delay ( after which MOH is played )
     */
    static int getMusicOnHoldDelayMiliseconds() {
        return getBridgeConfiguration().getMusicOnHoldDelayMiliseconds();
    }

    /**
     * Returns true if inbound calls go to an auto attendant.
     *
     * @return
     */
    static boolean isInboundCallsRoutedToAutoAttendant() {
        return accountManager.getBridgeConfiguration()
                .isInboundCallsRoutedToAutoAttendant();
    }

    /**
     * Place where you want to direct calls ( assuming that you have such a
     * place ).
     *
     * @return - the address of auto attendant.
     */
    static String getAutoAttendantName() {
        return accountManager.getBridgeConfiguration().getAutoAttendantName();
    }

    /**
     * The XML rpc client connection to the symmitron.
     *
     * @param address -
     *            the address ( extracted from the Via header).
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
            try {
                Thread.sleep(100);
            } catch (Exception ex) {
            }
        }
        return symmitronClient;
    }
    public static HashSet<String> getSupportedTransports() {
       return Gateway.supportedTransports ;
     }
    
    /**
     * The set of codecs handled by the park server.
     *
     * @return
     */
    static HashSet<Integer> getParkServerCodecs() {
        return getBridgeConfiguration().getParkServerCodecs();
    }

    /**
     * Log an internal error and potentially throw a runtime exception ( if
     * debug is enabled).
     *
     * @param errorString
     */
    static void logInternalError(String errorString, Exception exception) {
        if (logger.isDebugEnabled()) {
            logger.fatal(errorString, exception);
            throw new SipXbridgeException(errorString, exception);
        } else {
            logger.fatal(errorString, exception);
        }

    }

    static void logInternalError(String errorString) {
        if (logger.isDebugEnabled()) {
            logger.fatal(errorString);
            throw new SipXbridgeException(errorString);
        } else {
            /*
             * Log our stack trace for analysis.
             */
            logger.fatal(errorString, new Exception());
        }
    }

    /**
     *
     * This method must be called before SSL connection is initialized.
     */
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
            throw new SipXbridgeException(ex);
        }
    }

    /**
     * The main method for the Bridge.
     *
     * @param args
     */
    public static void main(String[] args) throws Exception {
        try {
            String command = System.getProperty("sipxbridge.command", "start");
            Gateway.parseConfigurationFile();
            if (command.equals("start")) {
                Gateway.start();
            } else if (command.equals("configtest")) {
                Gateway.configtest();
            } else if (command.equals("stop")) {
                Gateway.exit();
            } else {
                System.err.println("Bad option " + command);
                System.exit(-1);
            }

        } catch (Throwable ex) {
            System.err.println("SipXbridge : Exception caught while running");
            if (logger != null) {
                logger.fatal("SipXbridge : Fatal error caught ", ex);
            }
            ex.printStackTrace(System.err);
            System.exit(-1);
        }

    }

   

}
