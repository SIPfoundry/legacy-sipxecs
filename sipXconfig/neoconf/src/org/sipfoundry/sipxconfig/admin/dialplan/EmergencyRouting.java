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
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.gateway.Gateway;

/**
 * EmergencyRouting - Only used in special cases, calls are directed immediately to
 * gateway for phones that cannot call directly. 
 */
public class EmergencyRouting extends BeanWithId {
    private Gateway m_defaultGateway;
    private String m_externalNumber;

    private Collection<RoutingException> m_exceptions = new ArrayList<RoutingException>();

    public void addException(RoutingException exception) {
        exception.setEmergencyRouting(this);
        m_exceptions.add(exception);
    }

    public void removeException(RoutingException exception) {
        m_exceptions.remove(exception);
        exception.setEmergencyRouting(null);
    }

    // getters and setters
    public Gateway getDefaultGateway() {
        return m_defaultGateway;
    }

    public void setDefaultGateway(Gateway defaultGateway) {
        m_defaultGateway = defaultGateway;
    }

    public Collection<RoutingException> getExceptions() {
        return m_exceptions;
    }

    public void setExceptions(Collection<RoutingException> exceptions) {
        m_exceptions = exceptions;
    }

    public String getExternalNumber() {
        return m_externalNumber;
    }

    public void setExternalNumber(String externalNumber) {
        m_externalNumber = externalNumber;
    }

    /**
     * Converts emergency dialing object to the list of dialing rules. It is used to generate
     * proper authorization rules.
     * 
     * @return list of DialingRules
     */
    public List asDialingRulesList() {
        ArrayList rules = new ArrayList();
        if (null != m_defaultGateway && StringUtils.isNotBlank(m_externalNumber)) {
            DialingRule rule = createDialRule(m_defaultGateway, m_externalNumber);
            rules.add(rule);
        }
        for (RoutingException re : m_exceptions) {
            Gateway gateway = re.getGateway();
            String externalNumber = re.getExternalNumber();
            if (null != gateway && StringUtils.isNotBlank(externalNumber)) {
                DialingRule rule = createDialRule(gateway, externalNumber);
                rules.add(rule);
            }
        }
        return rules;
    }

    /**
     * Creates custom dial rule: one gateway, fixed pattern, no translation, no permission
     * 
     * @param gateway rule gateway
     * @param externalNumber rule dial patter
     * @return a newly create dial rule
     */
    private DialingRule createDialRule(Gateway gateway, String externalNumber) {
        CustomDialingRule rule = new CustomDialingRule();
        rule.setName("caller sensitive emergency routing");
        rule.setCallPattern(new CallPattern(StringUtils.EMPTY, CallDigits.FIXED_DIGITS));
        rule.setGateways(Collections.singletonList(gateway));
        rule.setDialPatterns(Collections.singletonList(new DialPattern(externalNumber, 0)));
        return rule;
    }

    public void removeGateways(Collection gatewayIds) {
        Collection<RoutingException> exceptions = new HashSet<RoutingException>(getExceptions());
        for (Iterator i = gatewayIds.iterator(); i.hasNext();) {
            Object id = i.next();
            if (m_defaultGateway != null && m_defaultGateway.getId().equals(id)) {
                m_defaultGateway = null;
            }
            for (Iterator<RoutingException> j = exceptions.iterator(); j.hasNext();) {
                RoutingException re = j.next();
                Gateway gateway = re.getGateway();
                if (gateway != null && gateway.getId().equals(id)) {
                    re.setGateway(null);
                    j.remove();
                }
            }
        }
    }
}
