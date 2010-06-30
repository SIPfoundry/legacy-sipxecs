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

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.collections.Transformer;
import org.apache.commons.collections.functors.ChainedTransformer;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.CoreContextImpl;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventPublisher;
import org.sipfoundry.sipxconfig.service.ConfigFileActivationManager;
import org.springframework.beans.factory.annotation.Required;

/**
 * Use hibernate to perform database operations
 */
public class SettingDaoImpl extends SipxHibernateDaoSupport implements SettingDao {
    private static final String RESOURCE_PARAM = "resource";
    private static final String NAME_PARAM = "name";
    private static final String GROUP_ID = "groupId";
    private static final String BRANCH = "branch";

    private DaoEventPublisher m_daoEventPublisher;

    private ConfigFileActivationManager m_configFileManager;

    public void setDaoEventPublisher(DaoEventPublisher daoEventPublisher) {
        m_daoEventPublisher = daoEventPublisher;
    }

    @Required
    public void setRlsConfigFilesActivator(ConfigFileActivationManager configFileManager) {
        m_configFileManager = configFileManager;
    }

    public Group getGroup(Integer groupId) {
        return (Group) getHibernateTemplate().load(Group.class, groupId);
    }

    public boolean deleteGroups(Collection<Integer> groupIds) {
        BeanWithId.IdToBean idToBean = new BeanWithId.IdToBean(getHibernateTemplate(), Group.class);
        Transformer publishDelete = new Transformer() {
            public Object transform(Object input) {
                m_daoEventPublisher.publishDelete(input);
                return input;
            }
        };
        Transformer loadAndPublish = ChainedTransformer.getInstance(idToBean, publishDelete);
        boolean affectAdmin = false;
        Group adminGroup = getGroupByName(User.GROUP_RESOURCE_ID, CoreContextImpl.ADMIN_GROUP_NAME);
        List groups = new ArrayList(groupIds.size());
        for (Integer groupId : groupIds) {
            Group group = loadGroup(groupId);
            if (group != adminGroup) {
                groups.add(loadAndPublish.transform(groupId));
            } else {
                affectAdmin = true;
            }
        }
        m_configFileManager.activateConfigFiles();
        getHibernateTemplate().deleteAll(groups);
        return affectAdmin;
    }

    public void storeValueStorage(ValueStorage storage) {
        getHibernateTemplate().saveOrUpdate(storage);
    }

    public ValueStorage loadValueStorage(Integer storageId) {
        return (ValueStorage) getHibernateTemplate().load(ValueStorage.class, storageId);
    }

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

        m_configFileManager.activateConfigFiles();
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

    public List<Group> getGroupsByString(String resource, String groupString, boolean saveNew) {
        if (StringUtils.isBlank(groupString)) {
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

    public Group loadGroup(Integer id) {
        return (Group) getHibernateTemplate().load(Group.class, id);
    }

    public List<Group> getGroups(String resource) {
        List<Group> groups = getHibernateTemplate().findByNamedQueryAndNamedParam("groupsByResource",
                RESOURCE_PARAM, resource);
        return groups;
    }

    public Map<Integer, Long> getGroupMemberCountIndexedByGroupId(Class groupOwner) {
        String query = "select g.id, count(*) from " + groupOwner.getName() + " o join o.groups g group by g.id";
        List<Object[]> l = getHibernateTemplate().find(query);
        Map<Integer, Long> members = asMap(l);

        return members;
    }

    public Map<Integer, Long> getBranchMemberCountIndexedByBranchId(Class branchOwner) {
        String query = "select b.id, count(*) from " + branchOwner.getName() + " o join o.branch b group by b.id";
        List<Object[]> l = getHibernateTemplate().find(query);
        Map<Integer, Long> members = asMap(l);

        return members;
    }

    public Map<Integer, Long> getGroupBranchMemberCountIndexedByBranchId(Class branchOwner) {
        String query = "select g.branch.id, count(*) from " + branchOwner.getName() + " o join "
            + "o.groups g where o.branch = null group by g.branch.id";
        List<Object[]> l = getHibernateTemplate().find(query);
        Map<Integer, Long> members = asMap(l);

        return members;
    }

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
}
