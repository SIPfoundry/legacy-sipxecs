/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.intercom;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.IntercomRule;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.EntityDeleteListener;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class IntercomManagerImpl extends SipxHibernateDaoSupport implements IntercomManager,
        BeanFactoryAware {

    public static final String CONTEXT_BEAN_NAME = "intercomManagerImpl";

    private BeanFactory m_beanFactory;

    private DialPlanActivationManager m_dialPlanActivationManager;

    public Intercom newIntercom() {
        return (Intercom) m_beanFactory.getBean(Intercom.class.getName());
    }

    /**
     * Return an Intercom instance. Create one if none exist. Throw UserException if more than one
     * Intercom instance exists, since the caller is assuming that there can be at most one
     * Intercom instance.
     */
    public Intercom getIntercom() {
        List intercoms = loadIntercoms();
        int numIntercoms = intercoms.size();
        if (numIntercoms == 0) {
            return newIntercom();
        } else if (numIntercoms == 1) {
            return (Intercom) intercoms.get(0);
        } else {
            throw new UserException("Expecting at most 1 Intercom, but there are " + numIntercoms);
        }
    }

    public void saveIntercom(Intercom intercom) {
        getHibernateTemplate().saveOrUpdate(intercom);
        m_dialPlanActivationManager.replicateDialPlan(true);
    }

    public List<Intercom> loadIntercoms() {
        return getHibernateTemplate().loadAll(Intercom.class);
    }

    /**
     * Remove all intercoms - mostly used for testing
     */
    public void clear() {
        HibernateTemplate template = getHibernateTemplate();
        Collection intercoms = template.loadAll(Intercom.class);
        template.deleteAll(intercoms);
    }

    /**
     * Return the intercom associated with a phone, through the groups the phone belongs to, or
     * null if there is no intercom for the phone. There should be at most one intercom for any
     * phone. If there is more than one, then return the first intercom found.
     */
    public Intercom getIntercomForPhone(Phone phone) {
        Set phoneGroups = phone.getGroups();
        for (Intercom intercom : loadIntercoms()) {
            Set intercomGroups = intercom.getGroups();
            if (CollectionUtils.containsAny(phoneGroups, intercomGroups)) {
                return intercom;
            }
        }
        return null;
    }

    /** Return a list of dialing rules, one for each intercom configuration */
    public List<DialingRule> getDialingRules() {
        List<Intercom> intercoms = loadIntercoms();
        List<DialingRule> rules = new ArrayList<DialingRule>(intercoms.size());
        for (Intercom intercom : intercoms) {
            DialingRule rule = new IntercomRule(intercom);
            rules.add(rule);
        }
        return rules;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public void setDialPlanActivationManager(DialPlanActivationManager dialPlanActivationManager) {
        m_dialPlanActivationManager = dialPlanActivationManager;
    }

    private class OnGroupDelete extends EntityDeleteListener<Group> {
        public OnGroupDelete() {
            super(Group.class);
        }

        protected void onEntityDelete(Group group) {
            List<Intercom> intercoms = loadIntercoms();
            for (Intercom intercom : intercoms) {
                Set<Group> groups = intercom.getGroups();
                if (groups.remove(group)) {
                    getHibernateTemplate().update(intercom);
                }
            }
        }

    }

    public EntityDeleteListener<Group> createGroupDeleteListener() {
        return new OnGroupDelete();
    }
}
