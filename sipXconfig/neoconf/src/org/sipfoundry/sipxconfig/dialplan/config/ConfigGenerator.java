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
import java.util.List;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.DialingRuleProvider;
import org.sipfoundry.sipxconfig.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.paging.PagingContext;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.rls.Rls;
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

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        // Need to include all areas that provide addresses to dialing rules.
        //
        // NOTE: Before adding another item, in some cases it's more appropriate for
        // other contexts could announce changes in their system affect DialPlanContext.FEATURE
        //
        if (!request.applies(ProxyManager.FEATURE, DialPlanContext.FEATURE, LocalizationContext.FEATURE,
                PagingContext.FEATURE, Rls.FEATURE, LocationsManager.FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);
        String domainName = manager.getDomainManager().getDomainName();
        for (Location location : locations) {
            if (!manager.getFeatureManager().isFeatureEnabled(ProxyManager.FEATURE, location)) {
                continue;
            }
            File dir = manager.getLocationDataDirectory(location);
            Writer[] writers = new Writer[] {
                new FileWriter(new File(dir, "mappingrules.xml")),
                new FileWriter(new File(dir, "authrules.xml")),
                new FileWriter(new File(dir, "fallbackrules.xml")),
                new FileWriter(new File(dir, "forwardingrules.xml"))
            };

            try {
                RulesFile[] files = new RulesFile[] {
                    (RulesFile) m_beanFactory.getBean("mappingRules"),
                    (RulesFile) m_beanFactory.getBean("authRules"),
                    (RulesFile) m_beanFactory.getBean("fallbackRules"),
                    (RulesFile) m_beanFactory.getBean("forwardingRules")
                };

                for (RulesFile file : files) {
                    file.setLocation(location);
                    file.setDomainName(domainName);
                }

                generateXml(files, location);

                for (int i = 0; i < files.length; i++) {
                    files[i].write(writers[i]);
                }

            } finally {
                for (int i = 0; i < writers.length; i++) {
                    IOUtils.closeQuietly(writers[i]);
                }
            }
        }
    }

    private void generateXml(RulesFile[] files, Location location) {
        // Get rules from dialing rule providers and the dial plan
        List<IDialingRule> rules = new ArrayList<IDialingRule>();
        if (m_dialingRuleProvider != null) {
            rules.addAll(m_dialingRuleProvider.getDialingRules(location));
        }
        rules.addAll(m_planContext.getGenerationRules(location));

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
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }
}
