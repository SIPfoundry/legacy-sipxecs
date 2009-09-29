/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxconfig.branch;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import static java.util.Collections.singletonList;

import org.hibernate.Criteria;
import org.hibernate.Session;
import org.hibernate.criterion.Criterion;
import org.hibernate.criterion.Restrictions;
import org.sipfoundry.sipxconfig.admin.NameInUseException;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.orm.hibernate3.HibernateCallback;

public class BranchManagerImpl extends SipxHibernateDaoSupport<Branch> implements BranchManager,
        ApplicationContextAware {

    private static final String BRANCH = "branch";

    private static final String NAME_PROP_NAME = "name";

    private AliasManager m_aliasManager;

    private ApplicationContext m_applicationContext;

    private SettingDao m_settingDao;

    @Required
    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }

    @Override
    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }

    @Required
    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    @Override
    public Branch getBranch(Integer branchId) {
        return load(Branch.class, branchId);
    }

    public Branch getBranch(String branchName) {
        return loadBranchByUniqueProperty(NAME_PROP_NAME, branchName);
    }

    public void saveBranch(Branch branch) {
        // Check for duplicate names or extensions before saving the branch
        String name = branch.getName();
        if (!m_aliasManager.canObjectUseAlias(branch, name)) {
            throw new NameInUseException(BRANCH, name);
        }

        getHibernateTemplate().saveOrUpdate(branch);
    }

    /**
     * deletes the given branch and publishes event to trigger user location replication if branch
     * contains users (event contains branch name)
     */
    public void deleteBranch(Branch branch) {
        deleteBranches(singletonList(branch.getId()));
    }

    /**
     * deletes the given branches and publishes a single event to trigger user location
     * replication if any branch contains users (event contains names for branches with users)
     */
    public void deleteBranches(Collection<Integer> branchIds) {
        Map<Integer, Long> branchMemberCount = m_settingDao.getBranchMemberCountIndexedByBranchId(User.class);
        List<Integer> branchWithUsersIds = new ArrayList<Integer>();

        Collection<Branch> branches = new ArrayList<Branch>(branchIds.size());
        for (Integer id : branchIds) {
            Branch branch = getBranch(id);
            branches.add(branch);
            Long memberCount = branchMemberCount.get(id);
            if (memberCount != null && memberCount > 0) {
                branchWithUsersIds.add(branch.getId());
            }
        }

        getHibernateTemplate().deleteAll(branches);

        if (branchWithUsersIds.size() > 0) {
            m_applicationContext.publishEvent(new BranchesWithUsersDeletedEvent(branchWithUsersIds));
        }
    }

    public List<Branch> getBranches() {
        List<Branch> branches = getHibernateTemplate().loadAll(Branch.class);
        return branches;
    }

    private Branch loadBranchByUniqueProperty(String propName, String propValue) {
        final Criterion expression = Restrictions.eq(propName, propValue);

        HibernateCallback callback = new HibernateCallback() {
            public Object doInHibernate(Session session) {
                Criteria criteria = session.createCriteria(Branch.class).add(expression);
                return criteria.list();
            }
        };

        List branches = getHibernateTemplate().executeFind(callback);
        Branch branch = (Branch) DaoUtils.requireOneOrZero(branches, expression.toString());

        return branch;
    }

    @Override
    public void clear() {
        removeAll(Branch.class);
    }

}
