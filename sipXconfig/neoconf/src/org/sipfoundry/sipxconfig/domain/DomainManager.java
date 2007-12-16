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

import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleProvider;

public interface DomainManager extends DialingRuleProvider {

    static final String CONTEXT_BEAN_NAME = "domainManager";

    Domain getDomain();

    String getAuthorizationRealm();

    void initialize();

    void saveDomain(Domain domain);

    void replicateDomainConfig();
    
    static class DomainNotInitializedException extends RuntimeException {
        DomainNotInitializedException() {
            super("System was not initialized properly");
        }
    }
}
