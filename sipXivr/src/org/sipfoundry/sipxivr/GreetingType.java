/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * 
 */

package org.sipfoundry.sipxivr;

public enum GreetingType {
    NONE("none"), 
    STANDARD("standard"), 
    OUT_OF_OFFICE("outofoffice"), 
    EXTENDED_ABSENCE("extendedabsence");
    
    private String m_id;
    
    GreetingType(String id) {
        m_id = id;
    }
    
    public String getId() {
        return m_id;
    }
    
    public static GreetingType valueOfById(String id) {
        for (GreetingType greeting : GreetingType.values()) {
            if (greeting.getId().equals(id)) {
                return greeting;
            }
        }
        throw new IllegalArgumentException("id not recognized " + id);
    }
}