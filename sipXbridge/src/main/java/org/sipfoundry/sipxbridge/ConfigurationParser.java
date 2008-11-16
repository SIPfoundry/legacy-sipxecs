/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import org.apache.commons.digester.Digester;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.SimpleLayout;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

/**
 * The Parser for the configuration file that the bridge will use.
 * 
 * @author M. Ranganathan
 * 
 */
public class ConfigurationParser {
    private static final String BRIDGE_CONFIG = "sipxbridge-config/bridge-configuration";
    private static final String ITSP_CONFIG = "sipxbridge-config/itsp-account";

    static {
        Logger logger = Logger.getLogger(Digester.class);
        logger.addAppender(new ConsoleAppender( new SimpleLayout()));
        logger.setLevel(Level.OFF);
        logger = Logger.getLogger( "org.apache.commons.beanutils");
        logger.addAppender(new ConsoleAppender( new SimpleLayout()));
        logger.setLevel(Level.OFF);
    }

    /**
     * Add the digester rules.
     * 
     * @param digester
     */
    private static void addRules(Digester digester) {

        digester.addObjectCreate("sipxbridge-config", AccountManagerImpl.class);
        digester.addObjectCreate(BRIDGE_CONFIG, BridgeConfiguration.class);
        digester.addSetNext(BRIDGE_CONFIG, "setBridgeConfiguration");
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "external-address"),
                "setExternalAddress", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "external-port"),
                "setExternalPort", 0, new Class[] {
                    Integer.class
                });
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "local-address"),
                "setLocalAddress", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "local-port"),
                "setLocalPort", 0, new Class[] {
                    Integer.class
                });
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "stun-server-address"),
                "setStunServerAddress", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "stun-interval"),
                "setGlobalAddressRediscoveryPeriod", 0);

        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "global-address"),
                "setGlobalAddress", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "global-port"),
                "setGlobalPort", 0, new Class[] {
                    Integer.class
                });
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "xml-rpc-port"),
                "setXmlRpcPort", 0, new Class[] {
                    Integer.class
                });

        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "allowed-codec-name"),
                "setCodecName", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "sipx-proxy-domain"),
                "setSipxProxyDomain", 0);
        
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "sipx-proxy-transport"),
                "setSipxProxyTransport", 0);

        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG,
                "music-on-hold-support-enabled"), "setMusicOnHoldSupportEnabled", 0, new Class[] {
            Boolean.class
        });

        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "log-level"), "setLogLevel",
                0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "sip-keepalive-seconds"),
                "setSipKeepalive", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "media-keepalive-seconds"),
                "setMediaKeepalive", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "log-directory"),
                "setLogFileDirectory", 0);

        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG,
                "max-number-of-concurrent-calls"), "setMaxCalls", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "is-reinvite-supported"),
                "setReInviteSupported", 0, new Class[] {
                    Boolean.class
                });
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG,
                "route-inbound-calls-to-extension"), "setAutoAttendantName", 0, new Class[] {
            String.class
        });

        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "sipxrelay-host"),
                "setSymmitronHost", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "sipxrelay-xml-rpc-port"),
                "setSymmitronXmlRpcPort", 0);
        
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "sipx-proxy-port"),
                "setSipxProxyPort", 0);

        /*
         * ITSP configuration support parameters.
         */
        digester.addObjectCreate(ITSP_CONFIG, ItspAccountInfo.class);
        digester.addSetNext(ITSP_CONFIG, "addItspAccount");

       
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "outbound-proxy"),
                "setOutboundProxy", 0);

        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "inbound-proxy"),
                "setInboundProxy", 0);

        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "outbound-proxy-port"),
                "setProxyPort", 0, new Class[] {
                    Integer.class
                });
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "registration-interval"),
                "setRegistrationInterval", 0, new Class[] {
                    Integer.class
                });
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "outbound-transport"),
                "setOutboundTransport", 0);
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "proxy-domain"),
                "setProxyDomain", 0);
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "authentication-realm"),
                "setAuthenticationRealm", 0);
        digester
                .addCallMethod(String.format("%s/%s", ITSP_CONFIG, "user-name"), "setUserName", 0);
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "password"), "setPassword", 0);
       
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "rtp-keepalive-method"),
                "setRtpKeepaliveMethod", 0);

        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "sip-keepalive-method"),
                "setSipKeepaliveMethod", 0);

        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "use-global-addressing"),
                "setGlobalAddressingUsed", 0, new Class[] {
                    Boolean.class
                });
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "use-rport"), "setRportUsed",
                0, new Class[] {
                    Boolean.class
                });

        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "inbound-proyx-port"),
                "setInboundProxyPort", 0);

        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "register-on-initialization"),
                "setRegisterOnInitialization", 0, new Class[] {
                    Boolean.class
                });
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG,
                "use-registration-for-caller-id"), "setUseRegistrationForCallerId", 0,
                new Class[] {
                    Boolean.class
                });

        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG,
                "max-number-of-concurrent-calls"), "setMaxCalls", 0);

    }

    /**
     * Create an account manager structure and initialize it with the information pointed to by
     * the given URL.
     * 
     * @param url -- the rul from where to fetch the config file.
     * 
     * @return
     */

    AccountManagerImpl createAccountManager(String url) {
        // Create a Digester instance
        Digester digester = new Digester();

        digester.setSchema("file:schema/sipxbridge.xsd");

        addRules(digester);

        // Process the input file.
        try {
            InputSource inputSource = new InputSource(url);
            digester.parse(inputSource);
            AccountManagerImpl accountManagerImpl = (AccountManagerImpl) digester.getRoot();
            BridgeConfiguration bridgeConfiguration = accountManagerImpl.getBridgeConfiguration();
            if (bridgeConfiguration.getStunServerAddress() == null) {
                for (ItspAccountInfo itspAccountInfo : accountManagerImpl.getItspAccounts()) {
                    if (itspAccountInfo.isGlobalAddressingUsed())
                        throw new SAXException("Stun Server Address Not Specified");

                }
            }
            return (AccountManagerImpl) digester.getRoot();
        } catch (java.io.IOException ioe) {
            // Note that we do not have a debug file here so we need to print to stderr.
            ioe.printStackTrace(System.err);
            throw new GatewayConfigurationException("Intiialzation exception", ioe);
        } catch (org.xml.sax.SAXException se) {
            se.printStackTrace(System.err);
            throw new GatewayConfigurationException("Intiialzation exception", se);
        }

    }

}
