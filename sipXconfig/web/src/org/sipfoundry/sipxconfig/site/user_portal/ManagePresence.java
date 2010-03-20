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

import java.util.ArrayList;
import java.util.List;

import org.apache.tapestry.IComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.presence.PresenceServer;
import org.sipfoundry.sipxconfig.presence.PresenceServerImpl;
import org.sipfoundry.sipxconfig.presence.PresenceStatus;
import org.sipfoundry.sipxconfig.site.UserSession;

public abstract class ManagePresence extends SipxBasePage implements PageBeginRenderListener {

    @InjectObject(value = "spring:presenceServer")
    public abstract PresenceServer getPresenceServer();

    public abstract UserSession getUserSession();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject(value = "spring:acdContext")
    public abstract AcdContext getAcdContext();

    @InjectComponent(value = "actions")
    public abstract IComponent getActions();

    @Persist
    public abstract int getCurrentLocationId();

    public abstract void setCurrentLocationId(int id);

    @InjectObject(value = "spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @Persist
    public abstract LocationSelectionModel getLocationSelectionModel();

    public abstract void setLocationSelectionModel(LocationSelectionModel locationSelectionModel);


    public boolean isSignedIn() {
        User user = getCoreContext().loadUser(getUserSession().getUserId());
        AcdServer acdServer = getAcdContext().getAcdServerForLocationId(getCurrentLocationId());
        PresenceStatus status = getPresenceServer().getStatus(user, acdServer);
        return status == PresenceStatus.OPEN;
    }

    public void signIn() {
        User user = getCoreContext().loadUser(getUserSession().getUserId());
        AcdServer acdServer = getAcdContext().getAcdServerForLocationId(getCurrentLocationId());
        getPresenceServer().signIn(user, acdServer);
    }

    public void signOut() {
        User user = getCoreContext().loadUser(getUserSession().getUserId());
        AcdServer acdServer = getAcdContext().getAcdServerForLocationId(getCurrentLocationId());
        getPresenceServer().signOut(user, acdServer);
    }

    public void refresh(IRequestCycle request) {
        request.getPage();
    }

    public IComponent getActionBlock() {
        PresenceServerImpl server = (PresenceServerImpl) getPresenceServer();
        return server.isEnabled() ? getActions() : null;
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        if (isAcdServerPresentForThisUser()) {
            User user = getCoreContext().loadUser(getUserSession().getUserId());
            List acdServers = getAcdContext().getServers();
            if (getCurrentLocationId() == 0) {
                setCurrentLocationId(((AcdServer) acdServers.get(0)).getLocation().getId());
            }
            List<AcdServer> validAcdServers = new ArrayList<AcdServer>();
            for (int i = 0; i < acdServers.size(); i++) {
                if (getAcdContext().isUserAnAgentOnThisServer((AcdServer) acdServers.get(i), user)) {
                    validAcdServers.add((AcdServer) acdServers.get(i));
                }
            }

            setLocationSelectionModel(new LocationSelectionModel(validAcdServers));
        }
    }

    public boolean isAcdServerPresentForThisUser() {
        if  (0 < getAcdContext().getServers().size()) {
            List acdServers = getAcdContext().getServers();
            User user = getCoreContext().loadUser(getUserSession().getUserId());

            for (int i = 0; i < acdServers.size(); i++) {
                if (getAcdContext().isUserAnAgentOnThisServer((AcdServer) acdServers.get(i), user)) {
                    return true;
                }
            }
        }
        return false;
    }

    public static class LocationSelectionModel extends ObjectSelectionModel {
        LocationSelectionModel(List<AcdServer> acdServers) {
            List<Location> locations = new ArrayList<Location>();
            for (int i = 0; i < acdServers.size(); i++) {
                locations.add(acdServers.get(i).getLocation());
            }

            setCollection(locations);
            setLabelExpression("fqdn");
            setValueExpression("id");
        }
    }
}
