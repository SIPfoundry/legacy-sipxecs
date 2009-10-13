/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.sipxivr;

import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlTransient;
import javax.xml.bind.annotation.XmlValue;

@XmlRootElement(name = "activegreeting")
public class ActiveGreeting {
    private GreetingType m_greetingType = GreetingType.NONE;

    @XmlValue
    public String getActiveGreeting() {
        return m_greetingType.getId();
    }

    public void setActiveGreeting(String activeGreeting) {
        m_greetingType = GreetingType.valueOfById(activeGreeting);
    }
    
    @XmlTransient
    public GreetingType getGreetingType() {
        return m_greetingType;
    }
    
    public void setGreetingType(GreetingType greetingType) {
        m_greetingType = greetingType;
    }
}
