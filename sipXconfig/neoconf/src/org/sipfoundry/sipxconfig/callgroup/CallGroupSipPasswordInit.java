/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.callgroup;

import org.sipfoundry.sipxconfig.common.InitTaskListener;

/**
 * Upgrades DB by initializing SIP password field of all existing call groups.
 */
public class CallGroupSipPasswordInit extends InitTaskListener {
    private CallGroupContext m_callGroupContext;

    public void onInitTask(String task) {
        m_callGroupContext.generateSipPasswords();
    }

    public void setCallGroupContext(CallGroupContext callGroupContext) {
        m_callGroupContext = callGroupContext;
    }
}
