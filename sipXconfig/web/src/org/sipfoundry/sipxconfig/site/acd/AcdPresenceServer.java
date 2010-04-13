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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.collections.CollectionUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.presence.PresenceServer;
import org.sipfoundry.sipxconfig.presence.PresenceStatus;

public abstract class AcdPresenceServer extends BaseComponent implements PageBeginRenderListener {

    public abstract AcdContext getAcdContext();

    public abstract PresenceServer getPresenceServer();

    public abstract CoreContext getCoreContext();

    public abstract List getUsers();

    public abstract void setUsers(List users);

    public abstract List getAllowedUsers();

    public abstract void setAllowedUsers(List users);

    public abstract User getCurrentUser();

    public abstract SelectMap getSelections();

    public abstract int getCurrentLocationId();

    public abstract void setCurrentLocationId(int id);

    public abstract LocationsManager getLocationsManager();

    public abstract LocationSelectionModel getLocationSelectionModel();

    public abstract void setLocationSelectionModel(LocationSelectionModel locationSelectionModel);

    /**
     * acts as cache for user status so we do not get status twice, once for rewind and
     * once for render
     **/
    public abstract Map getUserStatuses();

    public abstract void setUserStatuses(Map statuses);

    public PresenceStatus getCurrentUserStatus() {
        User user = getCurrentUser();
        PresenceStatus status = (PresenceStatus) getUserStatuses().get(user);
        AcdServer acdServer = getAcdContext().getAcdServerForLocationId(getCurrentLocationId());
        if (status == null) {
            status = getPresenceServer().getStatus(user, acdServer);
            getUserStatuses().put(user, status);
        }
        return status;
    }

    public void refresh() {
        // empty
    }

    public void signIn() {
        PresenceServer presenceServer = getPresenceServer();
        User[] users = DaoUtils.loadBeansArrayByIds(getCoreContext(), User.class,
                getSelections().getAllSelected());
        AcdServer acdServer = getAcdContext().getAcdServerForLocationId(getCurrentLocationId());
        for (int i = 0; i < users.length; i++) {
            presenceServer.signIn(users[i], acdServer);
        }
    }

    public void signOut() {
        PresenceServer presenceServer = getPresenceServer();
        User[] users = DaoUtils.loadBeansArrayByIds(getCoreContext(), User.class, getSelections()
                .getAllSelected());
        AcdServer acdServer = getAcdContext().getAcdServerForLocationId(getCurrentLocationId());
        for (int i = 0; i < users.length; i++) {
            presenceServer.signOut(users[i], acdServer);
        }
    }

    public void pageBeginRender(PageEvent event) {
        if (!isAcdServerPresent()) {
            return;
        }

        List allowedUsers = getAllowedUsers();
        if (allowedUsers == null) {
            allowedUsers = getAcdContext().getUsersWithAgents();
            setAllowedUsers(allowedUsers);
        }

        List acdServers = getAcdContext().getServers();
        if (getCurrentLocationId() == 0) {
            setCurrentLocationId(((AcdServer) acdServers.get(0)).getLocation().getId());
        }

        Location location = getLocationsManager().getLocation(getCurrentLocationId());
        List usersOnThisServer = getAcdContext().getUsersWithAgentsForLocation(location);
        Collection<User> users = CollectionUtils.intersection(usersOnThisServer,
                allowedUsers);
        setUsers(Arrays.asList(users.toArray()));

        if (getUserStatuses() == null) {
            setUserStatuses(new HashMap(users.size()));
        }
        setLocationSelectionModel(new LocationSelectionModel(acdServers));
    }

    public boolean isAcdServerPresent() {
        return 0 != getAcdContext().getServers().size();
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
