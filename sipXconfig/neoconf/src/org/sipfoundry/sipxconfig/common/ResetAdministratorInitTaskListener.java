/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

/**
 * Resets the superadmin user credentials to superadmin and pin to blank
 */
public class ResetAdministratorInitTaskListener implements SystemTaskEntryPoint {
    private CoreContext m_coreContext;

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void runSystemTask(String[] args) {
        m_coreContext.createAdminGroupAndInitialUserTask();
    }
}
