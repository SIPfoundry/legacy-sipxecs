/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleProvider;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;
import org.springframework.beans.factory.annotation.Required;

/**
 * ConfigGenerator
 */
public class ConfigGenerator {
    public static final String BEAN_NAME = "dialPlanConfigGenerator";

    private RulesXmlFile m_mappingRules;
    private AuthRules m_authRules;
    private FallbackRules m_fallbackRules;
    private ForwardingRules m_forwardingRules;

    private DialingRuleProvider m_dialingRuleProvider;

    @Required
    public void setForwardingRules(ForwardingRules forwardingRules) {
        m_forwardingRules = forwardingRules;
    }

    @Required
    public void setAuthRules(AuthRules authRules) {
        m_authRules = authRules;
    }

    @Required
    public void setMappingRules(RulesXmlFile mappingRules) {
        m_mappingRules = mappingRules;
    }

    @Required
    public void setFallbackRules(FallbackRules fallbackRules) {
        m_fallbackRules = fallbackRules;
    }

    @Required
    public void setDialingRuleProvider(DialingRuleProvider dialingRuleProvider) {
        m_dialingRuleProvider = dialingRuleProvider;
    }

    private List< ? extends ConfigurationFile> getRulesFiles() {
        return Arrays.asList(m_mappingRules, m_authRules, m_fallbackRules, m_forwardingRules);
    }

    public void generate(DialPlanContext plan) {
        generateXmlFromDialingRules(plan);
    }

    private void generateXmlFromDialingRules(DialPlanContext plan) {
        // Get rules from dialing rule providers and the dial plan
        List<IDialingRule> rules = new ArrayList<IDialingRule>();
        if (m_dialingRuleProvider != null) {
            rules.addAll(m_dialingRuleProvider.getDialingRules());
        }
        rules.addAll(plan.getGenerationRules());

        m_mappingRules.begin();
        m_authRules.begin();
        m_fallbackRules.begin();
        m_forwardingRules.begin();

        for (IDialingRule rule : rules) {
            m_mappingRules.generate(rule);
            m_authRules.generate(rule);
            m_fallbackRules.generate(rule);
            m_forwardingRules.generate(rule);
        }

        m_mappingRules.end();
        m_authRules.end();
        m_fallbackRules.end();
        m_forwardingRules.end();
    }

    /**
     * Retrieves configuration file - only for testing.
     *
     * @param type type of the configuration file
     */
    final ConfigurationFile getConfigurationFileByName(String name) {
        for (ConfigurationFile file : getRulesFiles()) {
            if (name.equals(file.getName())) {
                return file;
            }
        }

        return null;
    }

    public void activate(Location location, SipxReplicationContext sipxReplicationContext) {
        for (ConfigurationFile file : getRulesFiles()) {
            sipxReplicationContext.replicate(location, file);
        }
    }
}
