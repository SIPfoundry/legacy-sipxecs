/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import org.springframework.context.ApplicationEvent;

public class ConferenceConfigReplicatedEvent extends ApplicationEvent {
    private String m_serviceUrl;

    public ConferenceConfigReplicatedEvent(Object source_, String serviceUrl) {
        super(source_);
        m_serviceUrl = serviceUrl;
    }
    
    public String getServiceUrl() {
        return m_serviceUrl;
    }    
}

