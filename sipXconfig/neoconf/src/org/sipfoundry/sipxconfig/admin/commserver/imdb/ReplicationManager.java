/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.setting.Group;

/**
 * Contains method that deal with the replication of files and of Replicable entities.
 */
public interface ReplicationManager {
    /**
     * Replicates file content to remote locations
     *
     * @param locations list of locations that will receive replicated file
     * @param file object representing file content
     *
     * @return true if the replication has been successful, false otherwise
     */
    boolean replicateFile(Location location, ConfigurationFile file);

    /**
     * Write to MongoDB (imdb.entity) a single entity. This method will replicate all DataSet
     * configured for this Replicable.
     *
     * @param entity
     */
    void replicateEntity(Replicable entity);

    /**
     * Replicate to MongoDB (imdb.entity) a single DataSet of this Replicable
     *
     * @param entity
     * @param ds
     */
    void replicateEntity(Replicable entity, DataSet... ds);

    /**
     * Removes from MongoDB (imdb.entity) a single Replicable entity
     *
     * @param entity
     */
    void removeEntity(Replicable entity);

    /**
     * Replicate into MongoDB (imdb.node) the Location.
     * Used for registrations.
     * Not to be confused with ServiceConfiguratorImpl.replicateLocation()
     *
     * @param location
     * @return
     */
    boolean replicateLocation(Location location);

    /**
     * Removes from MongoDB this Location.
     * @param location
     */
    void removeLocation(Location location);

    /**
     * Regenerates the MongoDB imdb.entity collection which holds the configuration
     * information.
     */
    boolean replicateAllData();

    /**
     * Regenerates a DataSet of all Replicable entities.
     * @param ds
     */
    void replicateAllData(DataSet ds);

    /**
     * Issues a resync command to the specified Location.
     * Used to tell remote Mongo slave to pickup the latest modifications.
     * @param location
     */
    void resyncSlave(Location location);

    /**
     * Adds this permission to entities that support permissions and do not have it. Used when
     * adding a permission with default value checked.
     */
    void addPermission(Permission perm);

    /**
     * Removes this permission from entities that have it. Used when deleting a permission.
     */
    void removePermission(Permission perm);

    /**
     * Replicate a {@link Group}. Uses parallel processing
     */
    void replicateGroup(Group group);

    /**
     * Replicate a {@link Branch}
     */
    void replicateBranch(Branch branch);

    /**
     * Deletes a branch. It uses mongo to retrieve members, rather than DaoUtils.
     * It is done like this b/c after a branch is deleted, the associations are removed,
     * so no members would be retrieved.
     */
    void deleteBranch(Branch branch);
    /**
     * Deletes a group. It uses mongo to retrieve members, rather than DaoUtils.
     * It is done like this b/c after a branch is deleted, the associations are removed,
     * so no members would be retrieved.
     */
    void deleteGroup(Group group);

    /**
     * Creates the "registrarnode" db in mongo's imdb database.
     * It contains data about the stunnels.
     * The reason it is public is that it needs
     * @param location
     */
    void registerTunnels(Location location);
}
