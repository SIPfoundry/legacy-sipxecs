/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import static org.apache.commons.lang.StringUtils.isBlank;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.CoreContextImpl;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.jdbc.core.JdbcTemplate;

/**
 * Use hibernate to perform database operations
 */
public class SettingDaoImpl extends SipxHibernateDaoSupport implements SettingDao {
    private static final String RESOURCE_PARAM = "resource";
    private static final String NAME_PARAM = "name";
    private static final String GROUP_ID = "groupId";
    private static final String BRANCH = "branch";
    private static final String SEMICOLON = ";";
    private static final String CLOSED_BRACKET = ")";
    private static final String WHERE_CLAUSE = "where schedule.group_id=";

    private JdbcTemplate m_jdbcTemplate;

    @Override
    public Group getGroup(Integer groupId) {
        return getHibernateTemplate().load(Group.class, groupId);
    }

    /*
     * (non-Javadoc)
     * Use plain sql for increased efficiency when deleting user groups.
     * A thing to note is that this method breaks the convention established by DaoEventDispatcher,
     * namely publish delete event first, then proceed with the actual delete. It will actually
     * manually delete from DB the group associations and the group and then the delete event is published.
     * In the case of groups now, only ReplicationTrigger will trigger the delete sequence, all other
     * event listeners that listened to group deletes were removed, and control moved in this method.
     * This was the price to pay for increased efficiency in saving large groups.
     */
    @Override
    public boolean deleteGroups(Collection<Integer> groupIds) {
        List<String> sqlUpdates = new ArrayList<String>();
        boolean affectAdmin = false;
        Group adminGroup = getGroupByName(User.GROUP_RESOURCE_ID, CoreContextImpl.ADMIN_GROUP_NAME);
        List<Group> groups = new ArrayList<Group>(groupIds.size());
        for (Integer groupId : groupIds) {
            Group group = loadGroup(groupId);
            if (!group.equals(adminGroup)) {
                groups.add(group);
                sqlUpdates.add("DELETE FROM user_group where group_id=" + group.getId() + SEMICOLON);
                sqlUpdates.add("DELETE FROM supervisor where group_id=" + group.getId() + SEMICOLON);
                sqlUpdates.add("update ring set schedule_id=null where schedule_id=(select schedule_id from schedule "
                        + WHERE_CLAUSE + group.getId() + CLOSED_BRACKET + SEMICOLON);
                sqlUpdates.add("delete from schedule_hours where schedule_id = (select schedule_id from schedule "
                        + WHERE_CLAUSE + group.getId() + CLOSED_BRACKET + SEMICOLON);
                sqlUpdates.add("delete from schedule where schedule.group_id= " + group.getId()
                        + SEMICOLON);
                sqlUpdates.add("delete from speeddial_group_button where speeddial_id=(select speeddial_id from "
                        + "speeddial_group where speeddial_group.group_id="
                        + group.getId() + CLOSED_BRACKET + SEMICOLON);
                sqlUpdates.add("delete from speeddial_group where group_id=" + group.getId() + SEMICOLON);
                sqlUpdates.add("DELETE FROM intercom_phone_group where group_id=" + group.getId() + SEMICOLON);
                sqlUpdates.add("delete from phone_group where group_id=" + group.getId() + SEMICOLON);
                sqlUpdates.add("delete from phonebook_member where group_id=" + group.getId() + SEMICOLON);
                sqlUpdates.add("delete from phonebook_consumer where group_id=" + group.getId() + SEMICOLON);
                sqlUpdates.add("DELETE FROM group_storage where group_id=" + group.getId() + SEMICOLON);
            } else {
                affectAdmin = true;
            }
        }
        if (!sqlUpdates.isEmpty()) {
            m_jdbcTemplate.batchUpdate(sqlUpdates.toArray(new String[sqlUpdates.size()]));
            getDaoEventPublisher().publishDeleteCollection(groups);
        }
        return affectAdmin;
    }

    @Override
    public void storeValueStorage(ValueStorage storage) {
        getHibernateTemplate().saveOrUpdate(storage);
    }

    @Override
    public ValueStorage loadValueStorage(Integer storageId) {
        return getHibernateTemplate().load(ValueStorage.class, storageId);
    }

    @Override
    public void saveGroup(Group group) {
        checkDuplicates(group);
        checkBranchValidity(group);
        assignWeightToNewGroups(group);
        if (!group.isNew()) {
            String origGroupName = (String) getOriginalValue(group, NAME_PARAM);
            if (!origGroupName.equals(group.getName()) && origGroupName.equals(CoreContextImpl.ADMIN_GROUP_NAME)) {
                throw new UserException("&msg.error.renameAdminGroup");
            }
        }
        getHibernateTemplate().saveOrUpdate(group);
    }

    @Override
    public void moveGroups(List<Group> groups, Collection<Integer> groupIds, int step) {
        DataCollectionUtil.moveByPrimaryKey(groups, groupIds.toArray(), step);
        for (int i = 0; i < groups.size(); i++) {
            // weight is position + 1 - for compatibility with old code
            groups.get(i).setWeight(i + 1);
        }
        getHibernateTemplate().saveOrUpdateAll(groups);
        getDaoEventPublisher().publishSave(groups);
    }

    void assignWeightToNewGroups(Group group) {
        if (group.isNew() && group.getWeight() == null) {
            GroupWeight weight = new GroupWeight();
            getHibernateTemplate().save(weight);
            group.setWeight(weight.getWeight());
            getHibernateTemplate().delete(weight); // delete not strictly nec.
        }
    }

    void checkDuplicates(Group group) {
        String[] params = new String[] {
            RESOURCE_PARAM, NAME_PARAM
        };
        Object[] values = new Object[] {
            group.getResource(), group.getName()
        };
        List objs = getHibernateTemplate().findByNamedQueryAndNamedParam("groupIdsWithNameAndResource", params,
                values);
        DaoUtils.checkDuplicates(group, objs, new DuplicateGroupException(group.getName()));
    }

    private void checkBranchValidity(Group group) {
        String[] params = new String[] {
            GROUP_ID, BRANCH
        };
        Object[] values = new Object[] {
            group.getId(), group.getBranch()
        };
        List objs = getHibernateTemplate().findByNamedQueryAndNamedParam("selectedBranchValid", params,
                values);
        if (objs.size() > 0) {
            throw new UserException("&branch.validity.error", group.getBranch().getName());
        }
    }

    private static class DuplicateGroupException extends UserException {
        private static final String ERROR = "A group with name: {0} already exists.";

        public DuplicateGroupException(String name) {
            super(ERROR, name);
        }
    }

    @Override
    public Group getGroupByName(String resource, String name) {
        String[] params = new String[] {
            RESOURCE_PARAM, NAME_PARAM
        };
        Object[] values = new Object[] {
            resource, name
        };
        String query = "groupsByResourceAndName";
        List groups = getHibernateTemplate().findByNamedQueryAndNamedParam(query, params, values);
        return (Group) DaoUtils.requireOneOrZero(groups, query);
    }

    @Override
    public List<Group> getGroupsByString(String resource, String groupString, boolean saveNew) {
        if (isBlank(groupString)) {
            return new ArrayList(0);
        }
        String[] groupNames = groupString.trim().split("\\s+");
        List<Group> groups = new ArrayList<Group>(groupNames.length);
        for (int i = 0; i < groupNames.length; i++) {
            Group g = getGroupByName(resource, groupNames[i]);
            if (g == null) {
                g = new Group();
                g.setResource(resource);
                g.setName(groupNames[i]);
                if (saveNew) {
                    saveGroup(g);
                }
            }
            groups.add(g);
        }

        return groups;
    }

    @Override
    public Group loadGroup(Integer id) {
        return getHibernateTemplate().load(Group.class, id);
    }

    @Override
    public List<Group> getGroups(String resource) {
        List<Group> groups = getHibernateTemplate().findByNamedQueryAndNamedParam("groupsByResource",
                RESOURCE_PARAM, resource);
        return groups;
    }

    @Override
    public Map<Integer, Long> getGroupMemberCountIndexedByGroupId(Class groupOwner) {
        String query = "select g.id, count(*) from " + groupOwner.getName() + " o join o.groups g group by g.id";
        List<Object[]> l = getHibernateTemplate().find(query);
        Map<Integer, Long> members = asMap(l);

        return members;
    }

    @Override
    public Map<Integer, Long> getBranchMemberCountIndexedByBranchId(Class branchOwner) {
        String query = "select b.id, count(*) from " + branchOwner.getName() + " o join o.branch b group by b.id";
        List<Object[]> l = getHibernateTemplate().find(query);
        Map<Integer, Long> members = asMap(l);

        return members;
    }

    @Override
    public Map<Integer, Long> getGroupBranchMemberCountIndexedByBranchId(Class branchOwner) {
        String query = "select g.branch.id, count(*) from " + branchOwner.getName() + " o join "
            + "o.groups g where o.branch = null group by g.branch.id";
        List<Object[]> l = getHibernateTemplate().find(query);
        Map<Integer, Long> members = asMap(l);

        return members;
    }

    @Override
    public Map<Integer, Long> getAllBranchMemberCountIndexedByBranchId(Class branchOwner) {
        Map<Integer, Long> mapBranch = getBranchMemberCountIndexedByBranchId(branchOwner);
        Map<Integer, Long> mapGroupBranch = getGroupBranchMemberCountIndexedByBranchId(branchOwner);
        for (Integer branchId : mapGroupBranch.keySet()) {
            Long noOwners1 = mapBranch.get(branchId);
            Long noOwners2 = mapGroupBranch.get(branchId);
            mapBranch.put(branchId, (noOwners1 == null ? 0 : noOwners1) + noOwners2);
        }

        return mapBranch;
    }

    public static Map asMap(List<Object[]> l) {
        Map m = new HashMap(l.size());
        for (int i = 0; i < l.size(); i++) {
            Object[] row = l.get(i);
            m.put(row[0], row[1]);
        }

        return m;
    }

    /**
     * Internal object, only used to generate group weights in DB neutral way
     */
    static class GroupWeight {

        private Integer m_weight = new Integer(-1);

        public Integer getWeight() {
            return m_weight;
        }

        public void setWeight(Integer weight) {
            m_weight = weight;
        }
    }

    @Override
    public Group getGroupCreateIfNotFound(String resourceId, String groupName) {
        Group g = getGroupByName(resourceId, groupName);
        if (g == null) {
            g = new Group();
            g.setResource(resourceId);
            g.setName(groupName);
            saveGroup(g);
        }
        return g;
    }

    public void setConfigJdbcTemplate(JdbcTemplate jdbcTemplate) {
        m_jdbcTemplate = jdbcTemplate;
    }

    @Override
    public void updateSettingName(String oldName, String newName) {
        String query = "update setting_value set path = '" + newName + "' where path = '" + oldName + "'";
        m_jdbcTemplate.execute(query);
    }
}
