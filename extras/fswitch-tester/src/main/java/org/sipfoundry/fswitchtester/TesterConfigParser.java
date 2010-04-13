/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.fswitchtester;

import org.apache.commons.digester.Digester;
import org.xml.sax.InputSource;


public class TesterConfigParser {
    private static String TESTER_CONFIG = "tester-config";
    private static String TESTER_CLIENT = "tester-config/tester-client";
    
    private static void addRules(Digester digester) {
        digester.addObjectCreate("tester-config", TesterConfig.class);
        digester.addCallMethod(String.format("%s/%s", TESTER_CONFIG,"sipx-proxy-domain"),
                "setSipxProxyDomain",0);
        digester.addCallMethod(String.format("%s/%s", TESTER_CONFIG,"sipx-proxy-address"),
                "setSipxProxyAddress",0);
     
        digester.addCallMethod(String.format("%s/%s",TESTER_CONFIG,"sipx-proxy-port"), 
                "setSipxProxyPort", 0,
                new Class[] { Integer.class });
        digester.addCallMethod(String.format("%s/%s",TESTER_CONFIG,"test-duration"), 
                "setTestDuration", 0,
                new Class[] { Integer.class });
        
        digester.addObjectCreate(TESTER_CLIENT, TesterClientConfig.class);
        digester.addSetNext(TESTER_CLIENT,"addTestClient");
        digester.addCallMethod(String.format("%s/%s", TESTER_CLIENT,"media-file"),
                "setMediaFile",0);
    
        digester.addCallMethod(String.format("%s/%s", TESTER_CLIENT,"conference-extension"),
                "setConferenceExtension",0);
        digester.addCallMethod(String.format("%s/%s", TESTER_CLIENT,"tester-host"),
                "setTesterHost",0);
        digester.addCallMethod(String.format("%s/%s", TESTER_CLIENT,"media-file"),
                "setMediaFile",0);
        digester.addCallMethod(String.format("%s/%s",TESTER_CLIENT,"tester-sip-base-port"), 
                "setTesterSipBasePort", 0,
        new Class[] { Integer.class });
        digester.addCallMethod(String.format("%s/%s",TESTER_CLIENT,"tester-rtp-base-port"), 
                "setTesterRtpBasePort", 0,
        new Class[] { Integer.class });
       
        digester.addCallMethod(String.format("%s/%s",TESTER_CLIENT,"user-agent-count"), 
                "setUserAgentCount", 0,     
        new Class[] { Integer.class });
        
        digester.addCallMethod(String.format("%s/%s", TESTER_CLIENT,"client-id"),
                "setClientId",0,     
                new Class[] { Integer.class });
        
       digester.addCallMethod(String.format("%s/%s",TESTER_CLIENT,"xml-rpc-port"), 
                "setXmlRpcPort", 0,     
       new Class[] { Integer.class });
       
       digester.addCallMethod(String.format("%s/%s",TESTER_CLIENT,"sender-count"), 
               "setSenderCount", 0,     
      new Class[] { Integer.class });
        
    }

    public TesterConfig parse(String fileName) {
        Digester digester = new Digester();
        digester.setSchema("file:schema/tester.xsd");
        addRules(digester);
        // Process the input file.
        String url = "file:" + fileName;
        try {
            InputSource inputSource = new InputSource(url);
            digester.parse(inputSource);
            return (TesterConfig) digester
                    .getRoot();
    
        } catch (Exception se) {
             se.printStackTrace(System.err);
             throw new FreeSwitchTesterException("exception parsing config file", se);
        }

    }

}
