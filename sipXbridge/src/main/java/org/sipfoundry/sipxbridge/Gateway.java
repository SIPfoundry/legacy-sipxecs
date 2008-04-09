/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxbridge;

import java.io.File;
import java.io.FileInputStream;
import java.net.InetAddress;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Properties;
import java.util.Timer;
import java.util.TimerTask;

import javax.sip.ClientTransaction;
import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.ListeningPoint;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.SipException;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.address.Address;
import javax.sip.address.Hop;
import javax.sip.address.SipURI;
import javax.sip.message.Response;

import net.java.stun4j.StunAddress;
import net.java.stun4j.StunException;
import net.java.stun4j.client.NetworkConfigurationDiscoveryProcess;
import net.java.stun4j.client.ResponseSequenceServer;
import net.java.stun4j.client.StunDiscoveryReport;

import org.apache.log4j.FileAppender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import org.apache.log4j.SimpleLayout;
import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServer;
import org.apache.xmlrpc.webserver.WebServer;
import org.sipfoundry.log4j.SipFoundryAppender;
import org.sipfoundry.log4j.SipFoundryLayout;
import org.xbill.DNS.Lookup;
import org.xbill.DNS.Record;
import org.xbill.DNS.SRVRecord;
import org.xbill.DNS.TextParseException;
import org.xbill.DNS.Type;

import gov.nist.javax.sip.SipStackExt;
import gov.nist.javax.sip.clientauthutils.*;

import junit.framework.TestCase;

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
     * Internal SIp Provider
     */
    private static SipProvider internalProvider;

    /*
     * External provider.
     */
    private static SipProvider externalProvider;

    /*
     * Back to back user agent manager.
     */
    private static CallControlManager backToBackUserAgentManager;

    /*
     * The Sipx proxy port.
     */
    private static int sipxProxyPort = 5060;

    /*
     * The sipx proxy address
     */
    private static String sipxProxyAddress;

    /*
     * The SIPX proxy transport.
     */
    private static String sipxProxyTransport = "udp";

    /*
     * This is a placeholder - to be replaced by STUN
     */
    private static String globalAddress;

    /*
     * The log file
     */
    protected static final String logFile = "/var/log/sipxpbx/sipxbridge.log";

    /*
     * The stun port
     */
    private static final int STUN_PORT = 3478;

    /*
     * A table of timers for re-registration
     */
    public static Timer timer;

    /*
     * The Music on hold URL
     */
    private static Address musicOnHoldAddress;

    /*
     * The From address of the gateway.
     */
    private static Address gatewayFromAddress;

    /*
     * The MOH URL
     */
    private static SipURI musicOnHoldUri;

    /*
     * THe Webserver for the xml rpc interface.
     */
    private static WebServer webServer;

    /*
     * The Gateway state.
     */
    public static GatewayState state = GatewayState.STOPPED;

    /*
     * The time for REGISTER requests.
     */
    private static final int MAX_REGISTRATION_TIMER = 10000;

    /**
     * This is a static class - nobody calls this method.
     * 
     */
    private Gateway() {

    }

    public static void setConfigurationFileName(String configFileName) {
        Gateway.configurationFile = configFileName;
    }

    /**
     * Start the xml rpc server.
     * 
     */
    public static void startXmlRpcServer() {
        try {

            logger.addAppender(new SipFoundryAppender(new SipFoundryLayout(),
                    logFile));

            ConfigurationParser parser = new ConfigurationParser();
            accountManager = parser.createAccountManager(configurationFile);
            BridgeConfiguration bridgeConfiguration = accountManager
                    .getBridgeConfiguration();
            InetAddress localAddr = InetAddress.getByName(Gateway
                    .getLocalAddress());

            webServer = new WebServer(Gateway.getXmlRpcWebServerPort(),
                    localAddr);

            PropertyHandlerMapping handlerMapping = new PropertyHandlerMapping();

            handlerMapping.addHandler("sipXbridge", SipXbridgeServer.class);

            XmlRpcServer server = webServer.getXmlRpcServer();

            server.setHandlerMapping(handlerMapping);
            webServer.start();

        } catch (Exception ex) {
            logger.error("Error starting xml rpc server", ex);
            ex.printStackTrace();
            throw new RuntimeException("Bad configuration file");
        }
    }

    public static void stopXmlRpcServer() {
        try {
            webServer.shutdown();
        } catch (Exception ex) {
            logger.error("Error stopping xml rpc server.", ex);
        }
    }

    public static void discoverPublicAddress()
            throws GatewayConfigurationException {
        try {
            BridgeConfiguration bridgeConfiguration = accountManager
                    .getBridgeConfiguration();
            String stunServerAddress = bridgeConfiguration
                    .getStunServerAddress();

            if (stunServerAddress != null) {

                // Todo -- deal with the situation when this port may be taken.
                StunAddress localStunAddress = new StunAddress(Gateway
                        .getLocalAddress(), 5091);

                StunAddress serverStunAddress = new StunAddress(
                        stunServerAddress, STUN_PORT);

                NetworkConfigurationDiscoveryProcess addressDiscovery = new NetworkConfigurationDiscoveryProcess(
                        localStunAddress, serverStunAddress);
                addressDiscovery.start();
                StunDiscoveryReport report = addressDiscovery
                        .determineAddress();
                System.out.println("Stun report = " + report);
                globalAddress = report.getPublicAddress().getSocketAddress()
                        .getAddress().getHostAddress();
                if (report.getPublicAddress().getPort() != 5091) {
                    System.out
                            .println("WARNING External port != internal port ");
                }

            }
        } catch (Exception ex) {
            throw new GatewayConfigurationException(
                    "Error discovering public address", ex);
        }
    }

    /**
     * Initialize the bridge.
     * 
     */
    public static void init() {
        try {

            Gateway.timer = new Timer();

            /*
             * We re-parse the file here because it can change when the bridge
             * is re-initialized.
             */
            ConfigurationParser parser = new ConfigurationParser();
            accountManager = parser.createAccountManager(configurationFile);
            BridgeConfiguration bridgeConfiguration = accountManager
                    .getBridgeConfiguration();
            InetAddress localAddr = InetAddress.getByName(Gateway
                    .getLocalAddress());

            authenticationHelper = ((SipStackExt) ProtocolObjects.sipStack)
                    .getAuthenticationHelper(accountManager,
                            ProtocolObjects.headerFactory);

            int externalPort = bridgeConfiguration.getExternalPort();
            String externalAddress = bridgeConfiguration.getExternalAddress();
            System.out.println("External Address:port = " + externalAddress
                    + ":" + externalPort);

            ListeningPoint externalUdpListeningPoint = ProtocolObjects.sipStack
                    .createListeningPoint(externalAddress, externalPort, "udp");
            ListeningPoint externalTcpListeningPoint = ProtocolObjects.sipStack
                    .createListeningPoint(externalAddress, externalPort, "tcp");

            externalProvider = ProtocolObjects.sipStack
                    .createSipProvider(externalUdpListeningPoint);

            int localPort = bridgeConfiguration.getLocalPort();
            String localIpAddress = bridgeConfiguration.getLocalAddress();
            System.out.println("Local Address:port = " + localIpAddress + ":"
                    + localPort);

            SipURI mohUri = accountManager.getBridgeConfiguration()
                    .getMusicOnHoldName() == null ? null
                    : ProtocolObjects.addressFactory.createSipURI(
                            accountManager.getBridgeConfiguration()
                                    .getMusicOnHoldName(), Gateway
                                    .getSipxProxyDomain());
            if (mohUri != null)  {
                musicOnHoldAddress = ProtocolObjects.addressFactory
                    .createAddress(mohUri);
            }
            
            String domain = Gateway.getSipxProxyDomain();
            gatewayFromAddress = ProtocolObjects.addressFactory
                    .createAddress(ProtocolObjects.addressFactory.createSipURI(
                            SIPXBRIDGE_USER, domain));

            ListeningPoint internalUdpListeningPoint = ProtocolObjects.sipStack
                    .createListeningPoint(localIpAddress, localPort, "udp");

            ListeningPoint internalTcpListeningPoint = ProtocolObjects.sipStack
                    .createListeningPoint(localIpAddress, localPort, "tcp");

            internalProvider = ProtocolObjects.sipStack
                    .createSipProvider(internalUdpListeningPoint);

            registrationManager = new RegistrationManager(getWanProvider());

            backToBackUserAgentManager = new CallControlManager();

            Hop hop = getSipxProxyHop();
            if (hop == null) {
                System.out
                        .println("Cannot resolve sipx proxy address check config ");
            }
            System.out.println("Proxy assumed to be running at 	" + hop);

        } catch (Exception ex) {
            throw new RuntimeException("Cannot initialize gateway", ex);
        }

    }

    public static AccountManagerImpl getAccountManager() {
        return Gateway.accountManager;

    }

    public static RegistrationManager getRegistrationManager() {
        return registrationManager;
    }

    public static AuthenticationHelper getAuthenticationHelper() {
        return authenticationHelper;
    }

    public static SipProvider getWanProvider() {
        return externalProvider;
    }

    public static SipProvider getLanProvider() {
        return internalProvider;
    }

    /**
     * @return the backToBackUserAgentManager
     */
    public static CallControlManager getCallControlManager() {
        return backToBackUserAgentManager;
    }

    /**
     * The local address of the gateway.
     * 
     * @return
     */

    public static String getLocalAddress() {
        return accountManager.getBridgeConfiguration().getLocalAddress();
    }

    /**
     * @return the sipxProxyAddress
     */
    public static HopImpl getSipxProxyHop() {
        try {
            /*
             * No caching of results here as the failover mechanism depends upon
             * DNS.
             */
            if (logger.isDebugEnabled())
                logger
                        .debug("getSipxProxyHop() : Looking up the following address "
                                + "_sip._"
                                + getSipxProxyTransport()
                                + "."
                                + getSipxProxyDomain());

            Record[] records = new Lookup("_sip._" + getSipxProxyTransport()
                    + "." + getSipxProxyDomain(), Type.SRV).run();
            if (records == null) {
                logger.debug("SRV record lookup returned null");
                Gateway.sipxProxyAddress = getSipxProxyDomain();
                // Could not look up the SRV record. Try the A record.
                return new HopImpl(getSipxProxyDomain(), getSipxProxyPort(),
                        getSipxProxyTransport());

            } else {
                SRVRecord record = (SRVRecord) records[0];
                int port = record.getPort();
                String resolvedName = record.getTarget().toString();
                // Sometimes the DNS (on linux) appends a "." to the end of the
                // record.
                // The library ought to strip this.
                if (resolvedName.endsWith(".")) {
                    resolvedName = resolvedName.substring(0, resolvedName
                            .lastIndexOf('.'));
                }
                Gateway.sipxProxyAddress = resolvedName;
                Gateway.sipxProxyPort = port;
                return new HopImpl(resolvedName, port, getSipxProxyTransport());
            }

        } catch (TextParseException ex) {
            logger
                    .error("Problem looking up proxy address -- stopping gateway");
            return null;
        }

    }

    /**
     * Get the sipx proxy address from the domain. This is determined by using
     * the DNS query above.
     * 
     * @return the sipx proxy address ( determined from DNS)
     */
    public static String getSipxProxyAddress() {
        return sipxProxyAddress;
    }

    /**
     * Get the sipx proxy port from the domain. This is determined by using the
     * DNS query above.
     * 
     * @return the sipx proxy domain.
     */
    public static int getSipxProxyPort() {
        return sipxProxyPort;
    }

    /**
     * The transport to use to talk to sipx proxy. This is registered in the DNS
     * srv.
     * 
     * @return the proxy transport
     */
    public static String getSipxProxyTransport() {
        return sipxProxyTransport;
    }

    /**
     * The Sipx proxy domain.
     * 
     * @return the sipx proxy domain name.
     */
    public static String getSipxProxyDomain() {
        return accountManager.getBridgeConfiguration().getSipxProxyDomain();
    }

    /**
     * 
     * @return the bridge log level.
     */
    public static String getLogLevel() {
        return accountManager.getBridgeConfiguration().getLogLevel();
    }

    /**
     * @return the upper bound of the port range we are allowing for RTP.
     * 
     */
    public static int getRtpPortRangeUpperBound() {

        return accountManager.getBridgeConfiguration().getRtpPortUpperBound();
    }

    /**
     * @return the lower bound of the port range for RTP.
     */
    public static int getRtpPortRangeLowerBound() {
        return accountManager.getBridgeConfiguration().getRtpPortLowerBound();
    }

    /**
     * Get the timeout for media.
     * 
     * @return
     */
    public static int getMediaKeepaliveMilisec() {
        return Gateway.accountManager.getBridgeConfiguration()
                .getMediaKeepalive();
    }

    /**
     * Get the MOH server Request URI.
     */
    public static SipURI getMusicOnHoldUri() {
        try {
            return accountManager.getBridgeConfiguration().getMusicOnHoldName() == null ? null
                    : ProtocolObjects.addressFactory.createSipURI(
                            accountManager.getBridgeConfiguration()
                                    .getMusicOnHoldName(), Gateway
                                    .getSipxProxyAddress());
        } catch (Exception ex) {
            logger.error("Unexpected exception creating Music On Hold URI", ex);
            throw new RuntimeException("Unexpected exception", ex);
        }
    }

    /**
     * @return the Music On Hold server Address.
     */
    public static Address getMusicOnHoldAddress() {
        return musicOnHoldAddress;
    }

    /**
     * Get the Gateway Address ( used in From Header ) of requests that
     * originate from the Gateway.
     * 
     * @return an address that can be used in the From Header of request that
     *         originate from the Gateway.
     */
    public static Address getGatewayFromAddress() {
        return Gateway.gatewayFromAddress;

    }

    public static void registerWithItsp() throws GatewayConfigurationException {
        System.out.println("------- REGISTERING--------");
        try {
            Gateway.accountManager.lookupItspAccountAddresses();
            Gateway.accountManager.startAuthenticationFailureTimers();

            for (ItspAccountInfo itspAccount : Gateway.accountManager
                    .getItspAccounts()) {
                if (itspAccount.isRegisterOnInitialization())
                    Gateway.registrationManager.sendRegistrer(itspAccount);
            }
            /*
             * Wait for successful registration.
             */
            for (int i = 0; i < MAX_REGISTRATION_TIMER / 1000; i++) {

                try {
                    Thread.sleep(1000);
                } catch (InterruptedException ex) {
                    throw new RuntimeException(
                            "Unexpected exception registering", ex);
                }
                /*
                 * Check to see if all accounts that need authentication have
                 * successfully registered.
                 */
                boolean allAuthenticated = true;
                for (ItspAccountInfo itspAccount : Gateway.accountManager
                        .getItspAccounts()) {
                    if (itspAccount.isRegisterOnInitialization()) {
                        if (itspAccount.getState() == AccountState.AUTHENTICATION_FAILED) {
                            Gateway.stop();
                            System.out
                                    .println("Authentication Failure with ITSP account check account info.");
                            throw new GatewayConfigurationException(
                                    "Authentication failure ");
                        } else if (itspAccount.getState() != AccountState.AUTHENTICATED) {
                            allAuthenticated = false;
                        }

                    }
                }

                if (allAuthenticated) {
                    System.out.println("---- SIPXBRIDGE READY ----");
                    Gateway.state = GatewayState.INITIALIZED;
                    return;
                } else {
                    Gateway.stop();
                }
            }
            throw new GatewayConfigurationException(
                    "Could not register with all accounts ");

        } catch (SipException ex) {
            throw new GatewayConfigurationException("Error in Registering ", ex);
        }
    }

    /**
     * Registers all listeners and starts everything.
     * 
     * @throws Exception
     */
    public static void startSipListener() throws GatewayConfigurationException {
        try {
            SipListenerImpl listener = new SipListenerImpl();
            getWanProvider().addSipListener(listener);
            getLanProvider().addSipListener(listener);
            ProtocolObjects.start();
        } catch (Exception ex) {
            throw new GatewayConfigurationException(
                    "Could not start gateway -- check configuration", ex);
        }

    }

    public static void start() throws GatewayConfigurationException {
        if (Gateway.getState() != GatewayState.STOPPED) {
            return;
        }
        Gateway.state = GatewayState.INITIALIZING;
        init();
        discoverPublicAddress();
        startSipListener();
        registerWithItsp();
    }

    public static synchronized void stop() {
        backToBackUserAgentManager.stop();
        timer.cancel();
        ProtocolObjects.stop();
        // TODO -- send deregistration to all accounts
        Gateway.state = GatewayState.STOPPED;
    }

    /**
     * Get the global address of igate ( this should be determined by Stun but
     * for now we hard code it ).
     * 
     * @return
     */
    public static String getGlobalAddress() {
        return globalAddress;
    }

    /**
     * Get the web server port.
     * 
     * @return the web server port.
     */
    public static int getXmlRpcWebServerPort() {
        return Gateway.accountManager.getBridgeConfiguration().getXmlRpcPort();
    }

    /**
     * Gets the current bridge status.
     * 
     * @return
     */
    public static GatewayState getState() {

        return Gateway.state;

    }

    /**
     * The main method for the Bridge.
     * 
     * @param args
     */
    public static void main(String[] args) throws Exception {
        Gateway.startXmlRpcServer();
        for (int i = 0; i < args.length; i++) {
            if ("-config".equals(args[i])) {
                Gateway.configurationFile = args[++i];
            } else if ("-initOnStartup".equals(args[i])) {
                start();

            }
        }

    }

}
