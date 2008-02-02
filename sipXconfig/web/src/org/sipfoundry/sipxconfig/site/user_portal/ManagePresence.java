/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import org.apache.tapestry.IComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectComponent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.presence.PresenceServer;
import org.sipfoundry.sipxconfig.presence.PresenceServerImpl;
import org.sipfoundry.sipxconfig.presence.PresenceStatus;
import org.sipfoundry.sipxconfig.site.UserSession;

public abstract class ManagePresence extends BasePage {

    public abstract PresenceServer getPresenceServer();

    public abstract UserSession getUserSession();

    public abstract CoreContext getCoreContext();
    
    @InjectComponent(value = "actions")
    public abstract IComponent getActions();

    public boolean isSignedIn() {
        User user = getCoreContext().loadUser(getUserSession().getUserId());
        PresenceStatus status = getPresenceServer().getStatus(user);
        return status == PresenceStatus.OPEN;
    }

    public void signIn() {
        User user = getCoreContext().loadUser(getUserSession().getUserId());
        getPresenceServer().signIn(user);
    }

    public void signOut() {
        User user = getCoreContext().loadUser(getUserSession().getUserId());
        getPresenceServer().signOut(user);
    }

    public void refresh(IRequestCycle request) {
        request.getPage();
    }

    public IComponent getActionBlock() {
        PresenceServerImpl server = (PresenceServerImpl) getPresenceServer();
        return server.isEnabled() ? getActions() : null;  
    }
}
