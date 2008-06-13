/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge.symmitron;

import org.apache.commons.digester.Digester;
import org.xml.sax.InputSource;

public class SymmitronConfigParser {
    private static final String BRIDGE_CONFIG = "sipxrelay-config";
  
    /**
     * Add the digester rules.
     * 
     * @param digester
     */
    private static void addRules(Digester digester) {
        digester.addObjectCreate("sipxrelay-config", SymmitronConfig.class);
        
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG,
        "xml-rpc-port"), "setXmlRpcPort", 0,
        new Class[] { Integer.class });
        
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG,
        "port-range"), "setPortRange", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG,
        "external-address"), "setExternalAddress", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG,
        "local-address"), "setLocalAddress", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG,
        "log-level"), "setLogLevel", 0);
        digester.addCallMethod(String.format("%s/%s", BRIDGE_CONFIG,
        "log-directory"), "setLogFileDirectory", 0);
    }
    
    public SymmitronConfig parse(String url) {
        // Create a Digester instance
        Digester digester = new Digester();
        digester.setSchema("file:schema/sipxrelay.xsd");

        addRules(digester);
        // Process the input file.
        try {
            InputSource inputSource = new InputSource(url);
            digester.parse(inputSource);
            return (SymmitronConfig) digester.getRoot();
        } catch (java.io.IOException ioe) {
            // Note that we do not have a debug file here so we need to print to stderr.
            ioe.printStackTrace(System.err);
            throw new RuntimeException("Intiialzation exception", ioe);
        } catch (org.xml.sax.SAXException se) {
             se.printStackTrace(System.err);
             throw new RuntimeException("Intiialzation exception", se);
        }

    }
  
}
