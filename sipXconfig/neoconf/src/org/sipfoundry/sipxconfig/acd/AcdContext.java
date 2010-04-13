/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd;

import java.io.Serializable;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.common.User;

public interface AcdContext extends AliasOwner, AliasProvider {
    public static final String CONTEXT_BEAN_NAME = "acdContext";

    List<AcdServer> getServers();

    boolean isAcdServerIdValid(int acdServerId);

    List getUsersWithAgents();

    List getUsersWithAgentsForLocation(Location location);

    void store(AcdComponent acdComponent);

    AcdServer newServer();

    /**
     * Used to notify that new ACD Service has been added to this location.
     * @param location to which new ACD Service has been added
     */
    void addNewServer(Location location);

    AcdLine newLine();

    AcdQueue newQueue();

    AcdAudio newAudio();

    void removeServers(Collection serversIds);

    void removeLines(Collection linesIds);

    void removeQueues(Collection queuesIds);

    void removeAgents(Serializable queueId, Collection agentsIds);

    void addUsersToQueue(Serializable queueId, Collection usersIds);

    AcdServer loadServer(Serializable serverId);

    AcdLine loadLine(Serializable id);

    AcdQueue loadQueue(Serializable id);

    AcdAgent loadAgent(Serializable id);

    void associate(Serializable lineId, Serializable queueId);

    void moveAgentsInQueue(Serializable queueId, Collection agentsIds, int step);

    void moveQueuesInAgent(Serializable agnetId, Collection queueIds, int step);

    /**
     * Removes all the servers, queues, lines and agents.
     */
    void clear();

    String getAudioServerUrl();

    /**
     * Migration task
     */
    void migrateOverflowQueues();

    void migrateLineExtensions();

    /**
     * @return true if ACD configuration disabled
     */
    boolean isEnabled();

    Collection<AcdQueue> getQueuesForUsers(AcdServer server, Collection<User> agents);

    void removeOverflowSettings(Collection overflowIds, String overflowType);

    void migrateAcdServers();

    AcdServer getAcdServerForLocationId(Integer locationId);

    boolean isUserAnAgentOnThisServer(AcdServer server, User user);
}
