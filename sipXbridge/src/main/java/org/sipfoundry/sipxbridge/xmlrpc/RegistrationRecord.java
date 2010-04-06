/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxbridge.xmlrpc;

import java.util.HashMap;
import java.util.Map;

/**
 * Registration record.
 */
public class RegistrationRecord {
    private String registeredAddress;
    private String registrationStatus;
    
    
    public RegistrationRecord (String address, String status) {
        registeredAddress = address;
        registrationStatus = status;
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
