package org.sipfoundry.siptester;

import org.apache.commons.digester.Digester;
import org.xml.sax.InputSource;

public class TestMapParser {
    private static final String TEST_MAPS = "test-maps";
    private static final String USER_MAPS = "user-maps";
    private static final String USER_MAP = "user-map";
    private static final String MAPS_TO = "maps-to";
    private static final String ADDRESS_MAP = "address-map";
    private static final String ADDRESS_MAPS = "address-maps";
    private static String testMapsUserMaps = TEST_MAPS + "/" + USER_MAPS;
    private static String testMapsAddressMaps = TEST_MAPS + "/" + ADDRESS_MAPS;
    
    public void addRules(Digester digester) {
        digester.addObjectCreate(TEST_MAPS, TestMap.class);
        digester.addObjectCreate(testMapsUserMaps + "/" + USER_MAP, NameValuePair.class);
        digester.addSetNext(testMapsUserMaps + "/" + USER_MAP, "addUserMap");
        digester.addObjectCreate(testMapsAddressMaps + "/" + ADDRESS_MAP, NameValuePair.class);
        digester.addSetNext(testMapsAddressMaps + "/" + ADDRESS_MAP, "addAddressMap");
     
        digester.addCallMethod(testMapsUserMaps + "/" + USER_MAP + "/" + "trace-user" , "setName", 0);
        digester.addCallMethod(testMapsUserMaps + "/" + USER_MAP + "/" + "maps-to" , "setValue", 0);
        digester.addCallMethod(testMapsAddressMaps + "/" + ADDRESS_MAP + "/" + "trace-address" , "setName", 0);
        digester.addCallMethod(testMapsAddressMaps + "/" + ADDRESS_MAP + "/" + "maps-to" , "setValue", 0);
        
    }
    
    public TestMap parse(String url) {
            Digester digester = new Digester();
            addRules(digester);
            InputSource inputSource = new InputSource(url);
            try {
                digester.parse(inputSource);
            } catch (Exception e) {
                throw new SipTesterException(e);
            }
            return (TestMap) digester.getRoot();
    }

}
