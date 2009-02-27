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
import java.util.List;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.ExtensionInUseException;
import org.sipfoundry.sipxconfig.admin.NameInUseException;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.dao.support.DataAccessUtils;

/**
 * DialPlanContextImpl is an implementation of DialPlanContext with hibernate support.
 */
public abstract class DialPlanContextImpl extends SipxHibernateDaoSupport implements BeanFactoryAware,
        DialPlanContext {
    private static final String DIALING_RULE_IDS_WITH_NAME_QUERY = "dialingRuleIdsWithName";
    private static final String VALUE = "value";
    private static final String DIALING_RULE = "dialing rule";

    private AliasManager m_aliasManager;

    private ListableBeanFactory m_beanFactory;

    /* delayed injection - working around circular reference */
    public abstract GatewayContext getGatewayContext();

    public abstract DialPlanActivationManager getDialPlanActivationManager();

    /**
     * Loads dial plan, creates a new one if none exist
     *
     * @return the single instance of dial plan
     */
    DialPlan getDialPlan() {
        List dialPlans = getHibernateTemplate().loadAll(DialPlan.class);
        return (DialPlan) DataAccessUtils.requiredSingleResult(dialPlans);
    }

    public boolean isInitialized() {
        List dialPlans = getHibernateTemplate().loadAll(DialPlan.class);
        return !dialPlans.isEmpty();
    }

    public boolean isDialPlanEmpty() {
        boolean empty = getHibernateTemplate().loadAll(DialPlan.class).isEmpty();
        return empty;
    }

    /**
     * @param rule new DialingRule to be added to the plan
     * @param position index of a new rule to be added, -1 means append the rule
     */
    public void addRule(int position, DialingRule rule) {
        if (!rule.isNew()) {
            throw new IllegalArgumentException("addRule method can be only called for new rules");
        }
        validateRule(rule);
        DialPlan dialPlan = getDialPlan();
        dialPlan.addRule(position, rule);
        getHibernateTemplate().saveOrUpdate(dialPlan);
    }

    public void storeRule(DialingRule rule) {
        validateRule(rule);

        // Save the rule. If it's a new rule then attach it to the dial plan first
        // and save it via the dial plan.
        if (rule.isNew()) {
            DialPlan dialPlan = getDialPlan();
            dialPlan.addRule(rule);
            getHibernateTemplate().saveOrUpdate(dialPlan);
        } else {
            getHibernateTemplate().saveOrUpdate(rule);
        }
        getDialPlanActivationManager().replicateDialPlan(true);
    }

    /**
     * Checks for duplicate names. Should be called before saving the rule.
     *
     * @param rule to be verified
     */
    private void validateRule(DialingRule rule) {
        String name = rule.getName();
        DaoUtils.checkDuplicatesByNamedQuery(getHibernateTemplate(), rule, DIALING_RULE_IDS_WITH_NAME_QUERY, name,
                new NameInUseException(DIALING_RULE, name));

        // For internal rules, check for alias collisions. Note: this method throws
        // an exception if it finds a duplicate.
        if (rule instanceof InternalRule) {
            checkAliasCollisionsForInternalRule((InternalRule) rule);
        }
        if (rule instanceof AttendantRule) {
            checkAliasCollisionsForAttendantRule((AttendantRule) rule);
        }
    }

    private void checkAliasCollisionsForInternalRule(InternalRule rule) {
        String voiceMailExtension = rule.getVoiceMail();
        if (!m_aliasManager.canObjectUseAlias(rule, voiceMailExtension)) {
            throw new ExtensionInUseException("voicemail", voiceMailExtension);
        }
    }

    private void checkAliasCollisionsForAttendantRule(AttendantRule ar) {
        String attendantExtension = ar.getExtension();
        if (!m_aliasManager.canObjectUseAlias(ar, attendantExtension)) {
            throw new ExtensionInUseException(DIALING_RULE, attendantExtension);
        }

        String aa = ar.getAttendantAliases();
        String[] aliases = AttendantRule.getAttendantAliasesAsArray(aa);

        for (int i = 0; i < aliases.length; i++) {
            String ruleAlias = aliases[i];
            if (!m_aliasManager.canObjectUseAlias(ar, ruleAlias)) {
                final String message = "Alias \"{0}\" is already in use.  "
                        + "Please choose another alias for this auto attendant.";
                throw new UserException(message, ruleAlias);
            }
        }
    }

    public List<DialingRule> getRules() {
        return getDialPlan().getRules();
    }

    /**
     * Gets all of the dialing rules using a particular gateway.
     *
     * @param gatewayId The ID of the gateway.
     * @return A List of the DialingRules for that gateway.
     */
    public List<DialingRule> getRulesForGateway(Integer gatewayId) {
        return getHibernateTemplate().findByNamedQueryAndNamedParam("dialingRulesByGatewayId", "gatewayId",
                gatewayId);
    }

    /**
     * Gets all of the dialing rules that can be added to a particular gateway.
     *
     * @param gatewayId The ID of the gateway
     * @return A List of the DialingRules that can be added to the gateway
     */
    public List<DialingRule> getAvailableRules(Integer gatewayId) {
        Collection<DialingRule> rules = getRules();
        List<DialingRule> usedRules = getRulesForGateway(gatewayId);
        List<DialingRule> availableRules = new ArrayList<DialingRule>();

        for (DialingRule rule : rules) {
            if (!usedRules.contains(rule) && rule.isGatewayAware()) {
                availableRules.add(rule);
            }
        }

        return availableRules;
    }

    public DialingRule getRule(Integer id) {
        return (DialingRule) getHibernateTemplate().load(DialingRule.class, id);
    }

    public void deleteRules(Collection<Integer> selectedRows) {
        DialPlan dialPlan = getDialPlan();
        dialPlan.removeRules(selectedRows);
        getHibernateTemplate().saveOrUpdate(dialPlan);
        getDialPlanActivationManager().replicateDialPlan(true);
    }

    public void duplicateRules(Collection<Integer> selectedRows) {
        DialPlan dialPlan = getDialPlan();
        List<DialingRule> rules = dialPlan.getRules();
        Collection<DialingRule> selectedRules = DataCollectionUtil.findByPrimaryKey(rules, selectedRows.toArray());
        for (DialingRule rule : selectedRules) {
            // Create a copy of the rule with a unique name
            DialingRule ruleDup = (DialingRule) duplicateBean(rule, DIALING_RULE_IDS_WITH_NAME_QUERY);

            rules.add(ruleDup);
        }
        DataCollectionUtil.updatePositions(rules);
        getHibernateTemplate().saveOrUpdate(dialPlan);
        getDialPlanActivationManager().replicateDialPlan(true);
    }

    public List<DialingRule> getGenerationRules() {
        DialPlan dialPlan = getDialPlan();
        return dialPlan.getGenerationRules();
    }

    public List<AttendantRule> getAttendantRules() {
        DialPlan dialPlan = getDialPlan();
        return dialPlan.getAttendantRules();
    }

    public void setOperator(AutoAttendant attendant) {
        DialPlan dialPlan = getDialPlan();
        dialPlan.setOperator(attendant);
        getHibernateTemplate().saveOrUpdate(dialPlan);
    }

    /**
     * Resets the flexible dial plan to factory defaults.
     *
     * Loads default rules definition from bean factory file.
     */
    public DialPlan resetToFactoryDefault(String dialPlanBeanName, AutoAttendant operator) {
        removeAll(DialingRule.class);
        removeAll(DialPlan.class);
        getGatewayContext().deleteVolatileGateways();
        getHibernateTemplate().flush();

        DialPlan newDialPlan = (DialPlan) m_beanFactory.getBean(dialPlanBeanName);
        newDialPlan.setOperator(operator);

        getHibernateTemplate().save(newDialPlan);
        // Flush the session to cause the delete to take immediate effect.
        // Otherwise we can get name collisions on dialing rules when we load the
        // default dial plan, causing a DB integrity exception, even though the
        // collisions would go away as soon as the session was flushed.
        getHibernateTemplate().flush();
        return newDialPlan;
    }

    /**
     * Reverts to default dial plan.
     */
    public String[] getDialPlanBeans() {
        return m_beanFactory.getBeanNamesForType(DialPlan.class, true, false);
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    @Required
    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }

    public void moveRules(Collection<Integer> selectedRows, int step) {
        DialPlan dialPlan = getDialPlan();
        dialPlan.moveRules(selectedRows, step);
        getHibernateTemplate().saveOrUpdate(dialPlan);
        getDialPlanActivationManager().replicateDialPlan(true);
    }

    /**
     * There can be multiple internal dialing rules and therefore multiple voicemail extensions,
     * but pick the most likely one.
     */
    public String getVoiceMail() {
        List<InternalRule> rules = DialPlan.getDialingRuleByType(getDialPlan().getRules(), InternalRule.class);
        if (rules.isEmpty()) {
            return InternalRule.DEFAULT_VOICEMAIL;
        }

        // return first, it's the most likely
        String voicemail = rules.get(0).getVoiceMail();
        return voicemail;
    }

    /**
     * Get first emergency rule that has a gateway w/o a route. This is the best guess at a
     * default emergency address. see: @link http://track.sipfoundry.org/browse/XCF-1883
     */
    public EmergencyInfo getLikelyEmergencyInfo() {
        List<EmergencyRule> rules = DialPlan.getDialingRuleByType(getDialPlan().getRules(), EmergencyRule.class);
        for (EmergencyRule rule : rules) {
            if (!rule.isEnabled()) {
                continue;
            }
            for (Gateway candidate : rule.getGateways()) {
                // by default phones cannot support sending to emergency host through
                // a gateway that has a route in between.
                // see http://list.sipfoundry.org/archive/sipx-dev/msg09644.html
                if (StringUtils.isBlank(candidate.getRoute())) {
                    int port = candidate.getAddressPort();
                    return new EmergencyInfo(candidate.getAddress(), port == 0 ? null : port, rule
                            .getEmergencyNumber());
                }
            }
        }

        return null;
    }

    public void removeGateways(Collection<Integer> gatewayIds) {
        List<DialingRule> rules = getRules();
        for (DialingRule rule : rules) {
            rule.removeGateways(gatewayIds);
            storeRule(rule);
        }
    }

    /**
     * Implement AliasOwner.isAliasInUse. DialPlanContextImpl owns aliases for auto attendants and
     * voicemail.
     */
    public boolean isAliasInUse(String alias) {
        if (getInternalRulesWithVoiceMailExtension(alias).size() > 0) {
            return true;
        }
        if (getAttendantRulesWithExtension(alias).size() > 0) {
            return true;
        }
        return isAutoAttendantAliasInUse(alias);
    }

    private boolean isAutoAttendantAliasInUse(String alias) {
        // Because auto attendant aliases are stored together in a space-delimited string,
        // we can't query the DB for individual aliases. However, there will be so few
        // of these aliases (one string per internal dialing rule) that we can simply load
        // all such alias strings and check them in Java.
        List<String> aliasStrings = getHibernateTemplate().findByNamedQuery("aaAliases");
        for (String aliasString : aliasStrings) {
            String[] aliases = AttendantRule.getAttendantAliasesAsArray(aliasString);
            if (ArrayUtils.contains(aliases, alias)) {
                return true;
            }
        }
        return false;
    }

    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        Collection bids = new ArrayList();

        Collection internalRules = getInternalRulesWithVoiceMailExtension(alias);
        bids.addAll(BeanId.createBeanIdCollection(internalRules, InternalRule.class));

        Collection attendantRules = getAttendantRulesWithExtension(alias);
        bids.addAll(BeanId.createBeanIdCollection(attendantRules, AttendantRule.class));

        bids.addAll(getBeanIdsOfRulesWithAutoAttendantAlias(alias));

        return bids;
    }

    private Collection<BeanId> getBeanIdsOfRulesWithAutoAttendantAlias(String alias) {
        Collection<Object[]> objs = getHibernateTemplate().findByNamedQuery("attendantRuleIdsAndAttendantAliases");
        Collection<BeanId> bids = new ArrayList<BeanId>();
        for (Object[] idAndAliases : objs) {
            Integer id = (Integer) idAndAliases[0];
            String aa = (String) idAndAliases[1];
            String[] aliases = AttendantRule.getAttendantAliasesAsArray(aa);
            if (ArrayUtils.contains(aliases, alias)) {
                bids.add(new BeanId(id, AttendantRule.class));
            }
        }
        return bids;
    }

    private Collection getInternalRulesWithVoiceMailExtension(String extension) {
        return getHibernateTemplate().findByNamedQueryAndNamedParam("internalRuleIdsWithVoiceMailExtension", VALUE,
                extension);
    }

    private Collection getAttendantRulesWithExtension(String extension) {
        return getHibernateTemplate().findByNamedQueryAndNamedParam("attendantRuleIdsWithExtension", VALUE,
                extension);
    }
}
