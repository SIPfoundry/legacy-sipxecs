/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dialplan.config;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.DialingRuleProvider;
import org.sipfoundry.sipxconfig.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.annotation.Required;

/**
 * ConfigGenerator
 */
public class ConfigGenerator implements ConfigProvider, BeanFactoryAware {
    public static final String BEAN_NAME = "dialPlanConfigGenerator";
    private DialingRuleProvider m_dialingRuleProvider;
    private DialPlanContext m_planContext;
    private ListableBeanFactory m_beanFactory;
    private MappingRules m_mappingRules;
    private AuthRules m_authRules;
    private FallbackRules m_fallbackRules;
    private ForwardingRules m_forwardingRules;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(DialPlanContext.FEATURE, LocalizationContext.FEATURE) || request.isFirstTime(this)) {
            return;
        }

        Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(
                ProxyManager.FEATURE);
        String domainName = manager.getDomainManager().getDomainName();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            Writer[] writers = new Writer[] {
                new FileWriter(new File(dir, "mappingrules.xml")),
                new FileWriter(new File(dir, "authrules.xml")),
                new FileWriter(new File(dir, "fallbackrules.xml")),
                new FileWriter(new File(dir, "forwardingrules.xml"))
            };

            RulesFile[] files = new RulesFile[] {
                    m_beanFactory.getBean(MappingRules.class),
                    m_beanFactory.getBean(AuthRules.class),
                    m_beanFactory.getBean(FallbackRules.class),
                    m_beanFactory.getBean(ForwardingRules.class),
            };

            for (RulesFile file : files) {
                file.setLocation(location);
                file.setDomainName(domainName);
            }

            generateXml(files);

            for (int i = 0; i < files.length; i++) {
                files[i].write(writers[i]);
                IOUtils.closeQuietly(writers[i]);
            }
        }
        request.firstTimeOver(this);
    }

    private void generateXml(RulesFile[] files) {
        // Get rules from dialing rule providers and the dial plan
        List<IDialingRule> rules = new ArrayList<IDialingRule>();
        if (m_dialingRuleProvider != null) {
            rules.addAll(m_dialingRuleProvider.getDialingRules());
        }
        rules.addAll(m_planContext.getGenerationRules());

        for (RulesFile file : files) {
            file.begin();
        }

        for (IDialingRule rule : rules) {
            for (RulesFile file : files) {
                file.generate(rule);
            }
        }

        for (RulesFile file : files) {
            file.end();
        }
    }

    @Required
    public void setDialingRuleProvider(DialingRuleProvider dialingRuleProvider) {
        m_dialingRuleProvider = dialingRuleProvider;
    }

    @Required
    public void setPlanContext(DialPlanContext planContext) {
        m_planContext = planContext;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) m_beanFactory;
    }

    public void setMappingRules(MappingRules mappingRules) {
        m_mappingRules = mappingRules;
    }

    public void setAuthRules(AuthRules authRules) {
        m_authRules = authRules;
    }

    public void setFallbackRules(FallbackRules fallbackRules) {
        m_fallbackRules = fallbackRules;
    }

    public void setForwardingRules(ForwardingRules forwardingRules) {
        m_forwardingRules = forwardingRules;
    }
}
