/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxconfig.branch;

import java.util.Collection;
import java.util.List;

import org.hibernate.Criteria;
import org.hibernate.Session;
import org.hibernate.criterion.Criterion;
import org.hibernate.criterion.Restrictions;
import org.sipfoundry.sipxconfig.admin.NameInUseException;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.orm.hibernate3.HibernateCallback;

public class BranchManagerImpl extends SipxHibernateDaoSupport<Branch> implements BranchManager {

    private static final String BRANCH = "branch";

    private static final String NAME_PROP_NAME = "name";

    private AliasManager m_aliasManager;

    @Required
    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
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

    public void deleteBranch(Branch branch) {
        getHibernateTemplate().delete(branch);
    }

    public void deleteBranches(Collection<Integer> branchIds) {
        removeAll(Branch.class, branchIds);
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
