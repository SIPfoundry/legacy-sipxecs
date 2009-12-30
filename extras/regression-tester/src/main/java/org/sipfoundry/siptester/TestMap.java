package org.sipfoundry.siptester;

import java.util.Hashtable;

public class TestMap {
    private Hashtable<String,String> userMaps = new Hashtable<String,String>();
    private Hashtable<String,String> addressMaps = new Hashtable<String,String>();
    
    public void addUserMap(NameValuePair mapElement) {
        System.out.println("mapElement " + mapElement.getName() + " " + mapElement.getValue());
        this.userMaps.put(mapElement.getName(), mapElement.getValue());
    }
    
    public void addAddressMap(NameValuePair mapElement) {
        this.addressMaps.put(mapElement.getName(), mapElement.getValue());
    }
    
    public String getMappedUser(String traceUser) {
        return this.userMaps.get(traceUser);
    }
    
    public String getMappedAddress(String traceAddress) {
        return this.addressMaps.get(traceAddress);
    }

}
