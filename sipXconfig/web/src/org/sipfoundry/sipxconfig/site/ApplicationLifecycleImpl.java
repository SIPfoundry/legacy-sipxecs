/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.engine.state.ApplicationStateManager;

/**
 * Logout copied from Vlib example, also see
 * http://thread.gmane.org/gmane.comp.java.tapestry.user/31641
 */
public class ApplicationLifecycleImpl implements ApplicationLifecycle {
    private boolean m_discardSession;

    private ApplicationStateManager m_stateManager;

    public void setStateManager(ApplicationStateManager stateManager) {
        m_stateManager = stateManager;
    }

    public void logout() {
        m_discardSession = true;
        if (m_stateManager.exists(UserSession.SESSION_NAME)) {
            UserSession userSession = (UserSession) m_stateManager.get(UserSession.SESSION_NAME);
            userSession.logout();
        }
    }

    public boolean getDiscardSession() {
        return m_discardSession;
    }
}
