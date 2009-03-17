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

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;



/**
 * Tapestry Visit object - session parameters for sipXconfig
 */
public class UserSession {
    public static final String SESSION_NAME = "userSession";

    /**
     * true if we want to display title bar and navigation false for testing and when embedding
     * pages in profilegen
     */
    private boolean m_navigationVisible = true;

    private boolean m_admin;
    
    private boolean m_supervisor;

    private boolean m_agent;

    /**
     * user that is currently logged in
     */
    private Integer m_userId;
    
    public boolean isNavigationVisible() {
        return m_navigationVisible;
    }

    public void setNavigationVisible(boolean navigationVisible) {
        m_navigationVisible = navigationVisible;
    }

    public boolean isAdmin() {
        return m_admin;
    }
    
    public boolean isSupervisor() {
        return m_supervisor;
    }

    public boolean isAgent() {
        return m_agent;
    }

    public Integer getUserId() {
        return m_userId;
    }
    
    /**
     * Loads a user from the specified core context
     * @param coreContext
     * @return The User object associated with this session
     */
    public User getUser(CoreContext coreContext) {
        return coreContext.loadUser(getUserId());
    }
    
    public boolean isLoggedIn() {
        return getUserId() != null;
    }

    public void login(Integer userId, boolean admin, boolean supervisor, boolean agent) {
        m_userId = userId;
        m_admin = admin;
        m_supervisor = supervisor;
        m_agent = agent;
    }
    
    public void logout() {
        m_userId = null;
    }
}
