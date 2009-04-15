/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.rest;

import java.util.HashMap;
import java.util.Map;

import org.restlet.Restlet;
import org.restlet.Router;

public class RestManager {
    public static final String CONTEXT_BEAN_NAME = "restManager";
    
    private Map<String, Restlet> m_resourceMappings = new HashMap<String, Restlet>();

    public void init(Router router) {
        for (String key : m_resourceMappings.keySet()) {
            router.attach(key, m_resourceMappings.get(key));
        }
    }
    
    public void setResourceMappings(HashMap<String, Restlet> resourceMappings) {
        m_resourceMappings = resourceMappings;
    }
}
