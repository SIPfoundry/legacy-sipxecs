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

import java.util.Collection;
import java.util.List;

import org.apache.commons.collections.CollectionUtils;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdQueue;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.acd.stats.AcdStatistics;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.site.UserSession;

public abstract class AcdSupervisorConsole extends PageWithCallback implements
        PageBeginRenderListener {
    public abstract Integer getAcdServerId();

    public abstract void setAcdServerId(Integer acdServerId);

    public abstract AcdServer getAcdServer();

    public abstract void setAcdServer(AcdServer server);

    public abstract String getTab();

    public abstract AcdStatistics getAcdStatistics();

    public abstract AcdContext getAcdContext();

    public abstract CoreContext getCoreContext();

    public abstract UserSession getUserSession();

    public abstract void setSupervisedUsers(Collection users);

    /**
     * Redirect to selecting ACD server, otherwise return expected tab
     */
    public abstract void setEffectiveTab(String tab);

    public void pageBeginRender(PageEvent event) {
        AcdContext context = getAcdContext();
        if (getAcdServerId() == null) {
            // make user pick a server iff there's more than 1
            List servers = context.getServers();
            if (servers.size() != 1) {
                setEffectiveTab("selectServer");
                return;
            }
            AcdServer server = (AcdServer) servers.get(0);
            setAcdServerId(server.getId());
        }

        setEffectiveTab(getTab());

        AcdServer server = getAcdServer();
        if (server == null) {
            server = context.loadServer(getAcdServerId());
            setAcdServer(server);

            AcdStatistics stats = getAcdStatistics();
            CoreContext core = getCoreContext();
            Integer userId = getUserSession().getUserId();
            User supervisor = core.loadUser(userId);

            List<User> supervisedUsers = core.getUsersThatISupervise(supervisor);
            List usersWithAgents = context.getUsersWithAgents();
            Collection<User> supervisedAgents = CollectionUtils.intersection(supervisedUsers,
                    usersWithAgents);

            stats.setUsers(supervisedAgents);
            Collection<AcdQueue> queues = context.getQueuesForUsers(server, supervisedAgents);

            stats.setQueues(queues);

            setSupervisedUsers(supervisedAgents);
        }
    }
}
