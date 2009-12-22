package org.sipfoundry.siptester;

import org.apache.commons.digester.Digester;
import org.xml.sax.InputSource;

public class TesterConfigParser {

    public static String TESTER_CONFIG = "tester-config";

    /**
     * Add the digester rules.
     * 
     * @param digester
     */
    private static void addRules(Digester digester) {
        digester.addObjectCreate(TESTER_CONFIG, SipTesterConfig.class);
        digester.addCallMethod(String.format("%s/%s", TESTER_CONFIG, "tester-ip-address"),
                "setTesterIpAddress",0);
        digester.addCallMethod(String.format("%s/%s", TESTER_CONFIG, "tester-base-port"),
                "setTesterBasePort",0);
        digester.addCallMethod(String.format("%s/%s", TESTER_CONFIG, "log-level"),
        "setLogLevel",0);
        digester.addCallMethod(String.format("%s/%s", TESTER_CONFIG, "test-user"),
                "addTestUser",0);
            

    }

    public SipTesterConfig parse(String url) {
        Digester digester = new Digester();
        addRules(digester);
        InputSource inputSource = new InputSource(url);
        try {
            digester.parse(inputSource);
        } catch (Exception e) {
            throw new SipTesterException(e);
        }
        return (SipTesterConfig) digester.getRoot();
    }

}
