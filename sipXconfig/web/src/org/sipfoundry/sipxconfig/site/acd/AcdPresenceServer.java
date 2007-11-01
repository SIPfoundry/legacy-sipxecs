/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.acd;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.presence.PresenceServer;
import org.sipfoundry.sipxconfig.presence.PresenceStatus;

public abstract class AcdPresenceServer extends BaseComponent implements PageBeginRenderListener {
    
    public abstract AcdContext getAcdContext();

    public abstract PresenceServer getPresenceServer();
    
    public abstract CoreContext getCoreContext();
    
    public abstract List getUsers();
    
    public abstract void setUsers(List users);
    
    public abstract User getCurrentUser();
    
    public abstract SelectMap getSelections();
    
    /** 
     * acts as cache for user status so we do not get status twice, once for rewind and 
     * once for render
     **/
    public abstract Map getUserStatuses();
    
    public abstract void setUserStatuses(Map statuses);
    
    public PresenceStatus getCurrentUserStatus() {
        User user = getCurrentUser();
        PresenceStatus status = (PresenceStatus) getUserStatuses().get(user);        
        if (status == null) {
            status = getPresenceServer().getStatus(user);
            getUserStatuses().put(user, status);
        }   
        return status;
    }
    
    public IPage refresh(IRequestCycle cycle) {
        return cycle.getPage();
    }
    
    public void signIn() {
        PresenceServer presenceServer = getPresenceServer();
        User[] users = DaoUtils.loadBeansArrayByIds(getCoreContext(), User.class, 
                getSelections().getAllSelected());        
        for (int i = 0; i < users.length; i++) {            
            presenceServer.signIn(users[i]);
        }
    }
    
    public void signOut() {
        PresenceServer presenceServer = getPresenceServer();
        User[] users = DaoUtils.loadBeansArrayByIds(getCoreContext(), User.class, getSelections()
                .getAllSelected());        
        for (int i = 0; i < users.length; i++) {            
            presenceServer.signOut(users[i]);
        }
    }
    
    public void pageBeginRender(PageEvent event) {
        List users = getUsers();
        if (users == null) {
            users = getAcdContext().getUsersWithAgents();
            setUsers(users);
        }
        
        if (getUserStatuses() == null) {
            setUserStatuses(new HashMap(users.size()));            
        }
    }
}
