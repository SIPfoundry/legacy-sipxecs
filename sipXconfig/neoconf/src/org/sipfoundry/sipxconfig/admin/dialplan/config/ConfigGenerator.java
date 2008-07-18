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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleProvider;
import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyRouting;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.AutoAttendantsConfig;
import org.springframework.beans.factory.annotation.Required;

/**
 * ConfigGenerator
 */
public class ConfigGenerator {
    public static final String BEAN_NAME = "dialPlanConfigGenerator";

    private MappingRules m_mappingRules;
    private AuthRules m_authRules;
    private FallbackRules m_fallbackRules;
    private ForwardingRules m_forwardingRules;
    private NatTraversalRules m_natTraversalRules;
    private AutoAttendantsConfig m_autoAttendantConfig;

    private DialingRuleProvider m_dialingRuleProvider;
    private EmergencyRoutingRules m_emergencyRoutingRules;
    private final List<AbstractConfigurationFile> m_attendantScheduleFiles = new ArrayList<AbstractConfigurationFile>();

    @Required
    public void setForwardingRules(ForwardingRules forwardingRules) {
        m_forwardingRules = forwardingRules;
    }

    @Required
    public void setNatTraversalRules(NatTraversalRules natTraversalRules) {
        m_natTraversalRules = natTraversalRules;
    }

    @Required
    public void setAuthRules(AuthRules authRules) {
        m_authRules = authRules;
    }

    @Required
    public void setMappingRules(MappingRules mappingRules) {
        m_mappingRules = mappingRules;
    }

    @Required
    public void setFallbackRules(FallbackRules fallbackRules) {
        m_fallbackRules = fallbackRules;
    }

    @Required
    public void setEmergencyRoutingRules(EmergencyRoutingRules emergencyRoutingRules) {
        m_emergencyRoutingRules = emergencyRoutingRules;
    }

    @Required
    public void setDialingRuleProvider(DialingRuleProvider dialingRuleProvider) {
        m_dialingRuleProvider = dialingRuleProvider;
    }

    @Required
    public void setAutoAttendantConfig(AutoAttendantsConfig autoAttendantConfig) {
        m_autoAttendantConfig = autoAttendantConfig;
    }

    private List< ? extends ConfigurationFile> getRulesFiles() {
        return Arrays.asList(m_mappingRules, m_authRules, m_fallbackRules, m_forwardingRules, m_natTraversalRules,
                m_autoAttendantConfig);
    }

    private void generate(EmergencyRouting er) {
        if (er == null) {
            return;
        }
        List<IDialingRule> rules = er.asDialingRulesList();
        for (IDialingRule rule : rules) {
            m_authRules.generate(rule);
        }

    }

    public void generate(DialPlanContext plan, EmergencyRouting er) {
        m_autoAttendantConfig.generate(plan);
        generateXmlFromDialingRules(plan, er);

        List<AttendantRule> attendantRules = plan.getAttendantRules();
        for (AttendantRule ar : attendantRules) {
            if (ar.isEnabled()) {
                AttendantScheduleFile file = new AttendantScheduleFile();
                file.generate(ar);
                m_attendantScheduleFiles.add(file);
            }
        }
    }

    /** Given the DialPlanContext and the EmergencyRouting, generate XML */
    private void generateXmlFromDialingRules(DialPlanContext plan, EmergencyRouting er) {
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

        generate(er);

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
     * Retrieves configuration file content as stream.
     *
     * Use only for preview, use write function to dump it to the file.
     *
     * @param type type of the configuration file
     */
    public String getFileContent(String name) {
        for (ConfigurationFile file : getRulesFiles()) {
            if (name.equals(file.getName())) {
                return file.getFileContent();
            }
        }

        return StringUtils.EMPTY;
    }

    public void activate(SipxReplicationContext sipxReplicationContext, String scriptsDirectory) {
        for (ConfigurationFile file : getRulesFiles()) {
            sipxReplicationContext.replicate(file);
        }
        for (AbstractConfigurationFile file : m_attendantScheduleFiles) {
            file.setDirectory(scriptsDirectory);
            sipxReplicationContext.replicate(file);
        }
    }

    public void activateEmergencyRules(SipxReplicationContext sipxReplicationContext, EmergencyRouting er,
            String domainName) {
        m_emergencyRoutingRules.generate(er, domainName);
        sipxReplicationContext.replicate(m_emergencyRoutingRules);
    }
}
