/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.apache.commons.collections.functors.InstanceofPredicate;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.ScheduledAttendant;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;

public class DialPlan extends BeanWithId {
    private List<DialingRule> m_rules = new ArrayList<DialingRule>();

    public List<DialingRule> getRules() {
        return m_rules;
    }

    public void setRules(List<DialingRule> rules) {
        m_rules = rules;
    }

    public void removeRule(DialingRule rule) {
        Object[] keys = new Object[] {
            rule.getId()
        };
        DataCollectionUtil.removeByPrimaryKey(m_rules, keys);
    }

    public void removeRules(Collection<Integer> ids) {
        DataCollectionUtil.removeByPrimaryKey(m_rules, ids.toArray());
    }

    public void addRule(DialingRule rule) {
        m_rules.add(rule);
    }

    public void addRule(int position, DialingRule rule) {
        if (position < 0) {
            m_rules.add(rule);
        } else {
            m_rules.add(0, rule);
        }
    }

    public void removeAllRules() {
        m_rules.clear();
    }

    /**
     * In some cases after DB upgrade we can end up with disjoint areas in the DB. If position
     * column has holes in it, hibernate retrieves array with null elements. We are cleaning it
     * here.
     *
     * @return boolean true if dial plan was changed and needs to be saved
     */
    public boolean removeEmptyRules() {
        boolean changed = false;
        for (Iterator<DialingRule> i = m_rules.iterator(); i.hasNext();) {
            if (i.next() == null) {
                i.remove();
                changed = true;
            }
        }
        return changed;
    }

    public void moveRules(Collection<Integer> selectedRows, int step) {
        DataCollectionUtil.moveByPrimaryKey(m_rules, selectedRows.toArray(), step);
    }

    public List<DialingRule> getGenerationRules() {
        List<DialingRule> generationRules = new ArrayList<DialingRule>();
        for (DialingRule rule : getRules()) {
            rule.appendToGenerationRules(generationRules);
        }

        VoicemailRedirectRule redirectRule = new VoicemailRedirectRule();
        redirectRule.appendToGenerationRules(generationRules);

        return generationRules;
    }

    /**
     * This function return all attendant rules contained in this plan.
     *
     * @return list of attendant rules, empty list if no attendant rules in this plan
     */
    public List<AttendantRule> getAttendantRules() {
        List<AttendantRule> attendantRules = new ArrayList<AttendantRule>();
        Predicate isAttendantRule = InstanceofPredicate.getInstance(AttendantRule.class);
        CollectionUtils.select(m_rules, isAttendantRule, attendantRules);
        return attendantRules;
    }

    /**
     * Run thru dialing rules and set relevant dial plans that take
     */
    public void setOperator(AutoAttendant operator) {
        List<AttendantRule> rules = getDialingRuleByType(m_rules, AttendantRule.class);
        for (AttendantRule ar : rules) {
            ScheduledAttendant sa = new ScheduledAttendant();
            sa.setAttendant(operator);
            ar.setAfterHoursAttendant(sa);
        }
    }

    static <T extends DialingRule> List<T> getDialingRuleByType(List<DialingRule> rulesCandidates, Class<T> c) {
        List<T> rules = new ArrayList<T>();
        for (DialingRule rule : rulesCandidates) {
            if (rule.getClass().isAssignableFrom(c)) {
                rules.add((T) rule);
            }
        }
        return rules;
    }
}
