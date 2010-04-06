/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.DataObjectSource;

/**
 * Database services for setting business objects
 */
public interface SettingDao extends DataObjectSource {

    /** common name found in spring file */
    String CONTEXT_NAME = "settingDao";

    void saveGroup(Group group);

    boolean deleteGroups(Collection<Integer> allSelected);

    void moveGroups(List<Group> groups, Collection<Integer> allSelected, int step);

    Group getGroupByName(String resource, String name);

    /**
     * Will create but not save groups that do not exist yet
     */
    List<Group> getGroupsByString(String resource, String groupString, boolean saveNew);

    Group getGroup(Integer id);

    List<Group> getGroups(String resource);

    Group loadGroup(Integer tagId);

    void storeValueStorage(ValueStorage storage);

    ValueStorage loadValueStorage(Integer storageId);

    /**
     * Get the number of members in each group
     *
     * @return map {groupId as Integer, count as Integer}
     */
    Map<Integer, Long> getGroupMemberCountIndexedByGroupId(Class groupOwner);

    /**
     * Get the number of members in each set branch
     *
     * @return map {branchId as Integer, count as Integer}
     */
    Map<Integer, Long> getBranchMemberCountIndexedByBranchId(Class branchOwner);

    /**
     * Get the number of members in each inherited group branch
     *
     * @return map {branchId as Integer, count as Integer}
     */
    Map<Integer, Long> getGroupBranchMemberCountIndexedByBranchId(Class branchOwner);

    /**
     * Get all number of members in each branch
     * (set branch number members + inherited group branch number members)
     *
     * @return map {branchId as Integer, count as Integer}
     */
    Map<Integer, Long> getAllBranchMemberCountIndexedByBranchId(Class branchOwner);

    /**
     * convienence: find and create group if not found
     */
    Group getGroupCreateIfNotFound(String resourceId, String groupName);
}
