/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import org.sipfoundry.sipxconfig.common.InitTaskListener;

/**
 * When system first starts up, create initial domain object w/default value(s)
 */
public class DomainInitializer extends InitTaskListener {
    private DomainManager m_domainManager;
    
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    /**
     * Initialize the domain on the first time that sipXconfig is run. If there is already an
     * existing domain instance returned by the domain manager, do not re-initialize
     */
    public void onInitTask(String task) {
        m_domainManager.initialize();
    }
}
