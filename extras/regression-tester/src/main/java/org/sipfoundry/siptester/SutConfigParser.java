package org.sipfoundry.siptester;

import org.apache.commons.digester.Digester;
import org.xml.sax.InputSource;

public class SutConfigParser {
    public static String SUT_CONFIG = "sut-config";
    public static String USER_AGENT = "user-agent";
    
    

    /**
     * Add the digester rules.
     * 
     * @param digester
     */
    private static void addRules(Digester digester) {
        digester.addObjectCreate(SUT_CONFIG, SutConfig.class);
      
        digester.addObjectCreate(String.format("%s/%s", SUT_CONFIG,USER_AGENT), SutUA.class);
        digester.addSetNext(String.format("%s/%s", SUT_CONFIG,USER_AGENT), "addSutUA");

        digester.addCallMethod(String.format("%s/%s/%s",SUT_CONFIG,USER_AGENT,"register"), "addRegistration",0);
        
        
        digester.addCallMethod(String.format("%s/%s/%s", SUT_CONFIG,USER_AGENT, "ip-address"),
                "setIpAddress",0);
        digester.addCallMethod(String.format("%s/%s/%s", SUT_CONFIG,USER_AGENT,"port"),
                "setPort",0);

    }

    public SutConfig parse(String url) {
        Digester digester = new Digester();
        addRules(digester);
        InputSource inputSource = new InputSource(url);
        try {
            digester.parse(inputSource);
        } catch (Exception e) {
            throw new SipTesterException(e);
        }
        return (SutConfig) digester.getRoot();
       
    }
}
