package org.sipfoundry.sipxbridge.xmlrpc;

import java.util.HashMap;
import java.util.Map;

/**
 * Registration record.
 */
public class RegistrationRecord {
    private String registeredAddress;
    private static String REGISTERED_ADDRESS = "registeredAddress";
    private String registrationStatus;
    private static final String REGISTRATION_STATUS = "registrationStatus";
    
    
    public static RegistrationRecord create(Map<String,String> map) {
        RegistrationRecord retval = new RegistrationRecord();
        retval.registeredAddress = map.get(REGISTERED_ADDRESS);
        retval.registrationStatus = map.get(REGISTRATION_STATUS);
        return retval;
    }
    
    
    public Map<String,String> getMap() {
        Map<String,String> retval = new HashMap<String,String>();
        retval.put(REGISTERED_ADDRESS, registeredAddress);
        retval.put(REGISTRATION_STATUS, registrationStatus);
        return retval;
        
    }
    /**
     * @param registeredAddress the registeredAddress to set
     */
    public void setRegisteredAddress(String registeredAddress) {
        this.registeredAddress = registeredAddress;
    }
    /**
     * @return the registeredAddress
     */
    public String getRegisteredAddress() {
        return registeredAddress;
    }
    /**
     * @param registrationStatus the registrationStatus to set
     */
    public void setRegistrationStatus(String registrationStatus) {
        this.registrationStatus = registrationStatus;
    }
    /**
     * @return the registrationStatus
     */
    public String getRegistrationStatus() {
        return registrationStatus;
    }
    
    

}
