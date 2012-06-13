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

import org.sipfoundry.sipxconfig.dialplan.DialingRuleProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.localization.Localization;
import org.sipfoundry.sipxconfig.setup.SetupManager;

public interface DomainManager extends DialingRuleProvider {
    public static final GlobalFeature FEATURE = new GlobalFeature("domain");

    static final String CONTEXT_BEAN_NAME = "domainManager";

    Domain getEditableDomain();

    Domain getDomain();

    String getAuthorizationRealm();

    void saveDomain(Domain domain);

    Localization getExistingLocalization();

    String getSharedSecret();

    String getDomainName();

    static class DomainNotInitializedException extends RuntimeException {
        DomainNotInitializedException() {
            super("System was not initialized properly");
        }
    }

    /**
     * For use in tests only.
     */
    public void setNullDomain();

    public boolean setup(SetupManager manager);
}
