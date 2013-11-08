/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver.imdb;

import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.setting.Group;

/**
 * Contains method that deal with the replication of files and of Replicable entities.
 */
public interface ReplicationManager {

    /**
     * Write to MongoDB (imdb.entity) a single entity. This method will replicate all DataSet
     * configured for this Replicable.
     *
     * @param entity
     */
    void replicateEntity(Replicable entity);

    /**
     * Removes from MongoDB (imdb.entity) a single Replicable entity
     *
     * @param entity
     */
    void removeEntity(Replicable entity);

    /**
     * Regenerates the MongoDB imdb.entity collection which holds the configuration
     * information.
     */
    void replicateAllData();

    /**
     * Regenerates a DataSet of all Replicable entities.
     * @param ds
     */
    void replicateAllData(DataSet ds);

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
     * Replicate a {@link Group}. Uses parallel processing
     */
    void replicateSpeedDialGroup(Group group);

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
     * Test that replication manager can replicate to database
     * @return
     */
    public boolean testDatabaseReady();

    public boolean replicateEntity(Replicable entity, DataSet field);
}
