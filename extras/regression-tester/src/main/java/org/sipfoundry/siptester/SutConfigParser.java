package org.sipfoundry.siptester;

import org.apache.commons.digester.Digester;
import org.xml.sax.InputSource;

public class SutConfigParser {
    public static final String SUT_CONFIG = "sut-config";
    public static final String ENDPOINT = "endpoint";
    public static final String EMULATED_PORT = "emulated-port";
    public static final String PORT = "port";
    public static final String PROXY = "proxy";
    public static final String BEHAVIOR = "behavior";
    public static final String IP_ADDRESS = "ip-address";
    
    

    /**
     * Add the digester rules.
     * 
     * @param digester
     */
    private static void addRules(Digester digester) {
        digester.addObjectCreate(SUT_CONFIG, TraceConfig.class); 
        digester.addObjectCreate(String.format("%s/%s", SUT_CONFIG,ENDPOINT), TraceEndpoint.class);
        digester.addSetNext(String.format("%s/%s", SUT_CONFIG,ENDPOINT), "addTraceEndpoint");      
        digester.addCallMethod(String.format("%s/%s/%s", SUT_CONFIG,ENDPOINT,IP_ADDRESS),
                "setIpAddress",0);
        digester.addCallMethod(String.format("%s/%s/%s", SUT_CONFIG,ENDPOINT,PORT),
                "setPort",0);
        digester.addCallMethod(String.format("%s/%s/%s", SUT_CONFIG,ENDPOINT,EMULATED_PORT),
                "setEmulatedPort",0);
        digester.addCallMethod(String.format("%s/%s/%s", SUT_CONFIG,ENDPOINT,BEHAVIOR),
                "setBehavior",0);

    }

    public TraceConfig parse(String url) {
        Digester digester = new Digester();
        addRules(digester);
        InputSource inputSource = new InputSource(url);
        try {
            digester.parse(inputSource);
        } catch (Exception e) {
            throw new SipTesterException(e);
        }
        return (TraceConfig) digester.getRoot();
       
    }
}
