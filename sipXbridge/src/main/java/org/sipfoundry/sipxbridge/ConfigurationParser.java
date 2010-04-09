/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import org.apache.commons.digester.Digester;
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
        logger.addAppender(new ConsoleAppender(new SimpleLayout()));
        logger.setLevel(Level.OFF);
        logger = Logger.getLogger("org.apache.commons.beanutils");
        logger.addAppender(new ConsoleAppender(new SimpleLayout()));
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
                "setGlobalPort", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "sipx-supervisor-host"),
                "setSipXSupervisorHost", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "sipx-supervisor-xml-rpc-port"),
                "setSipXSupervisorXmlRpcPort", 0, new Class[] {
                    Integer.class
                });
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "music-on-hold-address"),
                "setMusicOnHoldName", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "xml-rpc-port"),
                "setXmlRpcPort", 0, new Class[] {
                    Integer.class
                });

        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "sipx-proxy-domain"),
                "setSipxProxyDomain", 0);

        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "sipx-proxy-transport"),
                "setSipxProxyTransport", 0);

        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG,
                "music-on-hold-support-enabled"), "setMusicOnHoldSupportEnabled", 0, new Class[] {
            Boolean.class
        });

        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "music-on-hold-delay-miliseconds"),
                "setMusicOnHoldDelayMiliseconds", 0, new Class[] {
                    Integer.class
        });


        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "music-on-hold-supported-codecs"),
                "setMohSupportedCodecs", 0);

        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "music-on-hold-address"),
                "setMusicOnHoldName", 0);

        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "call-limit"),
                "setCallLimit", 0, new Class[] {
                    Integer.class
                });



        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "secure-xml-rpc"),
                "setSecure", 0, new Class[] {
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

        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "itsp-proxy-address"),
                "setOutboundProxy", 0);

        /*
         * Place where sipxbridge sends REGISTER requests.
         */
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "itsp-registrar-address"),
                "setInboundProxy", 0);

        /*
         * Place where sipxbridge sends INVITE
         */
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "itsp-proxy-listening-port"),
                "setOutboundProxyPort", 0, new Class[] {
                    Integer.class
                });

        /*
         * Port where sipxbridge sends REGISTER
         */
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "itsp-registrar-listening-port"),
                "setInboundProxyPort", 0, new Class[] {
                    Integer.class
                });

        /*
         * REGISTER interval.
         */
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "registration-interval"),
                "setRegistrationInterval", 0, new Class[] {
                    Integer.class
                });
        /*
         * Session timer interval.
         */
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "sip-session-timer-interval-seconds"),
                "setSipSessionTimerIntervalSeconds", 0, new Class[] {
                    Integer.class
        });
        /*
         * Transport the ITSP wants to see ( default to UDP )
         */
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "itsp-transport"),
                "setOutboundTransport", 0);

        /*
         * ITSP proxy domain.
         */
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "itsp-proxy-domain"),
                "setProxyDomain", 0);
        /*
         * sipxecs-lineid.
         */
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "sipxecs-lineids/sipxecs-lineid"),
                "setSipxecsLineId", 0);

        /*
         * Authentication user name
         */
        digester
                .addCallMethod(String.format("%s/%s", ITSP_CONFIG, "user-name"), "setUserName", 0);

        /*
         * Authentication password.
         */
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "authentication-user-name"), "setAuthenticationUserName", 0);

        /*
         * Authentication password.
         */
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "password"), "setPassword", 0);

        /*
         * Keep alive for RTP.
         */
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "rtp-keepalive-method"),
                "setRtpKeepaliveMethod", 0);

        /*
         * SIP Keep alive method.
         */
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "sip-keepalive-method"),
                "setSipKeepaliveMethod", 0);

        /*
         * Public addressing used on call setup signaling.
         */
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "use-global-addressing"),
                "setGlobalAddressingUsed", 0, new Class[] {
                    Boolean.class
                });

        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "enabled"),
                "setEnabled", 0, new Class[] {
                    Boolean.class
                });
        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "register-on-initialization"),
                "setRegisterOnInitialization", 0, new Class[] {
                    Boolean.class
                });

        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "strip-private-headers"),
                "setStripPrivateHeaders", 0, new Class[] {
                    Boolean.class
                });

        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "default-asserted-identity"),
                "setUseDefaultAssertedIdentity",0, new Class[] {
                    Boolean.class
                });

        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "asserted-identity"),
                "setCallerId", 0);


        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "default-preferred-identity"),
                "setUseDefaultPreferredIdentity",0, new Class[] {
                    Boolean.class
                });

        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "preferred-identity"),
                "setPreferredCallerId", 0);
        

        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "is-user-phone"),
                "setUserPhone", 0, new Class[] {
            Boolean.class
        });

        digester.addCallMethod(String.format("%s/%s", ITSP_CONFIG, "loose-route-invite"),
                "setAddLrRoute", 0, new Class[] {
            Boolean.class
        });
        
        digester.addCallMethod(String.format("%s/%s",ITSP_CONFIG,"always-relay-media"), "setAlwaysRelayMedia",0, new Class[] {
            Boolean.class
        });

    }

    /**
     * Create an account manager structure and initialize it with the information pointed to by
     * the given URL.
     *
     * @param url -- the rul from where to fetch the config file.
     *
     * @return
     */

    public AccountManagerImpl createAccountManager(String url) {
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
                    if (itspAccountInfo.isGlobalAddressingUsed() && bridgeConfiguration.getGlobalAddress() == null )
                        throw new SAXException("Need stun server address or public address to be specified in sipxbridge configuration.");

                }
            }
            return (AccountManagerImpl) digester.getRoot();
        } catch (java.io.IOException ioe) {
            // Note that we do not have a debug file here so we need to print to stderr.
            ioe.printStackTrace(System.err);
            throw new SipXbridgeException("Intiialzation exception", ioe);
        } catch (org.xml.sax.SAXException se) {
            se.printStackTrace(System.err);
            throw new SipXbridgeException("Intiialzation exception", se);
        }

    }

}
