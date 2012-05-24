/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.callgroup;

import org.sipfoundry.sipxconfig.setup.MigrationListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;

public class CallGroupSetup implements MigrationListener {
    private CallGroupContext m_callGroupContext;

    public void onInitTask(String task) {
        m_callGroupContext.generateSipPasswords();
    }

    public void setCallGroupContext(CallGroupContext callGroupContext) {
        m_callGroupContext = callGroupContext;
    }

    @Override
    public void migrate(SetupManager manager) {
        String id = "callgroup-password-init";
        if (manager.isTrue(id)) {
            // earlier versions didn't have sip passwords
            m_callGroupContext.generateSipPasswords();
            manager.setFalse(id);
        }
    }
}
