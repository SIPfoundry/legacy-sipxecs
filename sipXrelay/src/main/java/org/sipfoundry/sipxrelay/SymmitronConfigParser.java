/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrelay;

import org.apache.commons.digester.Digester;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.SimpleLayout;
import org.xml.sax.InputSource;

public class SymmitronConfigParser {

    /*
     * <?xml version="1.0" encoding="UTF-8"?> <nattraversal
     * xmlns="http://www.sipfoundry.org/sipX/schema/xml/nattraversalrules-00-00"> <info>
     * <state>enabled</state> <behindnat>true</behindnat> 
     * <publicaddress>68.33.195.46</publicaddress>
     * <proxyhostport>${MY_IP_ADDR}:${PROXY_SERVER_SIP_PORT}</proxyhostport>
     * <relayaggressiveness>Aggressive</relayaggressiveness> <concurrentrelays>50</concurrentrelays>
     * <port-range>30000:40000</port-range> <log-directory>/usr/local/sipxpbx</log-directory>
     * <log-level>DEBUG</log-level> <mediarelaypublicaddress>sipxpbx.example.com</mediarelaypublicaddress>
     * <mediarelaynativeaddress>sipxpbx.example.com</mediarelaynativeaddress>
     * <mediarelayxml-rpc-port>5080</mediarelayxml-rpc-port> </info> <localtopology>
     * <ipV4subnet>10.0.0.0/8</ipV4subnet> <ipV4subnet>172.16.0.0/12</ipV4subnet>
     * <ipV4subnet>192.168.0.0/16</ipV4subnet> <dnsWildcard>*.example.com</dnsWildcard>
     * </localtopology> </nattraversal>
     * 
     */
    private static final String BRIDGE_CONFIG = "nattraversal/info";
    
    static {
        Logger logger = Logger.getLogger(Digester.class);
        logger.setLevel(Level.OFF);
        logger.addAppender(new ConsoleAppender( new SimpleLayout()));
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
        
        digester.addObjectCreate("nattraversal", SymmitronConfig.class);
        
        

        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "mediarelayxml-rpc-port"),
                "setXmlRpcPort", 0, new Class[] {
                    Integer.class
                });
       
        
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, 
        "behindnat"), "setBehindNat", 0, new Class[] {
            Boolean.class
        });
        
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "rediscovery-time"),
                "setRediscoveryTime", 0, new Class[] {
                    Integer.class
                });
        
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "port-range"),
                "setPortRange", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "stun-server-address"),
                "setStunServerAddress", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "publicaddress"),
                "setPublicAddress", 0);
       
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "mediarelaynativeaddress"),
                "setLocalAddress", 0);
        
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "log-level"), "setLogLevel",
                0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "log-directory"),
                "setLogFileDirectory", 0);
        
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG,"secureXMLRPC"),
                "setUseHttps", 0, new Class[] {
            Boolean.class
        });
        
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "sipx-supervisor-host"),
                "setSipXSupervisorHost", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG, "sipx-supervisor-xml-rpc-port"),
                "setSipXSupervisorXmlRpcPort", 0, new Class[] {
                    Integer.class
                });
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG,"rejectStrayPackets"),
                "setRejectStrayPackets", 0, new Class[] {
            Boolean.class
        });
    }

    public SymmitronConfig parse(String url) {
        
            // Create a Digester instance
            Digester digester = new Digester();
             
            // digester.setSchema("file:schema/sipxrelay.xsd");

            addRules(digester);
            // Process the input file.
            try {
                InputSource inputSource = new InputSource(url);
                digester.parse(inputSource);
                return (SymmitronConfig) digester.getRoot();
            } catch (java.io.IOException ioe) {
                // Note that we do not have a debug file here so we need to print to stderr.
                ioe.printStackTrace(System.err);
                throw new SymmitronException("Intiialzation exception", ioe);
            } catch (org.xml.sax.SAXException se) {
                se.printStackTrace(System.err);
                throw new SymmitronException("Intiialzation exception", se);
            }
        

    }

}
