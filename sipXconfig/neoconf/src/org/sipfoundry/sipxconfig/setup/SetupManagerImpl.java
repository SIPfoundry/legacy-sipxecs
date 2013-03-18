/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.setup;


import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.search.IndexTrigger;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.jdbc.core.JdbcTemplate;

public class SetupManagerImpl implements SetupManager, ApplicationListener<ApplicationEvent>, BeanFactoryAware {
    private static final Log LOG = LogFactory.getLog(SetupManagerImpl.class);
    private ListableBeanFactory m_beanFactory;
    private Set<String> m_setupIds;
    private ConfigManager m_configManager;
    private FeatureManager m_featureManager;
    private CoreContext m_coreContext;
    private JdbcTemplate m_jdbcTemplate;
    private Set<SetupListener> m_setupListeners;
    private Set<MigrationListener> m_migrationListeners;
    private IndexTrigger m_indexTrigger;
    private boolean m_enabled = true;
    private boolean m_triggerConfigOnStartup = true;
    private boolean m_setup;
    private Context m_context;

    Set<String> getSetupIds() {
        if (m_setupIds == null) {
            List<String> setupIds = m_jdbcTemplate.queryForList("select setup_id from setup", String.class);
            m_setupIds = new HashSet<String>(setupIds);
        }
        return m_setupIds;
    }

    void saveSetupIds() {
        // by always deleting and reinserting we guarantee no duplicates even if
        // it's a bit efficient. Maybe you can improve should it be deemed nec.
        String remove = "delete from setup";
        StringBuilder update = new StringBuilder();
        for (String setupId : getSetupIds()) {
            if (update.length() == 0) {
                update.append("insert into setup values");
            } else {
                update.append(',');
            }
            update.append("('").append(setupId).append("')");
        }
        m_jdbcTemplate.batchUpdate(new String[] {
            remove, update.toString()
        });
    }

    @Override
    public ConfigManager getConfigManager() {
        return m_configManager;
    }

    @Override
    public FeatureManager getFeatureManager() {
        return m_featureManager;
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public Set<SetupListener> getSetupListeners() {
        if (m_setupListeners == null) {
            Map<String, SetupListener> beanMap = m_beanFactory.getBeansOfType(
                    SetupListener.class, false, false);
            m_setupListeners = new HashSet<SetupListener>(beanMap.values());
        }

        return m_setupListeners;
    }

    public Set<MigrationListener> getMigrationListeners() {
        if (m_migrationListeners == null) {
            Map<String, MigrationListener> beanMap = m_beanFactory.getBeansOfType(
                    MigrationListener.class, false, false);
            m_migrationListeners = new HashSet<MigrationListener>(beanMap.values());
        }

        return m_migrationListeners;
    }

    @Override
    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof ApplicationInitializedEvent) {
            setup(Context.APP_MAIN);
        }
    }

    @Override
    public void setup(Context c) {
        if (m_setup) {
            return;
        }

        m_context = c;
        if (m_enabled) {
            // There's no special detection when we're migrating v.s. starting up
            // only that migration tasks should run first so setup tasks can ensure
            // data is valid state before setup
            for (MigrationListener l : getMigrationListeners()) {
                l.migrate(this);
            }

            // fairly critical these are initialized first. Other listeners can use normal deps management
            m_configManager.getDomainManager().setup(this);
            m_configManager.getLocationManager().setup(this);
            m_indexTrigger.setup(this);
            m_coreContext.setup(this);
            List<SetupListener> again = new ArrayList<SetupListener>();
            Collection<SetupListener> todo = getSetupListeners();
            int lastCount = 0; // guard again infinite loop where setup listener never returns done
            while (!todo.isEmpty() && todo.size() != lastCount) {
                for (SetupListener l : todo) {
                    if (!l.setup(this)) {
                        again.add(l);
                    }
                }
                lastCount = todo.size();
                todo = again;
                LOG.info(todo.size() + " setup listeners will be called again");
                again = new ArrayList<SetupListener>();
            }
            if (todo.size() > 0) {
                LOG.error(todo.size() + " setup listeners never return 'true' signifying they were done");
            }
        }
        m_setup = true;

        if (m_triggerConfigOnStartup) {
            m_configManager.configureAllFeaturesEverywhere();
        }
    }

    public void setIndexTrigger(IndexTrigger indexTrigger) {
        m_indexTrigger = indexTrigger;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public void setJdbcTemplate(JdbcTemplate jdbcTemplate) {
        m_jdbcTemplate = jdbcTemplate;
    }

    /**
     * setup is not called for IntegrationTests
     */
    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public boolean isTriggerConfigOnStartup() {
        return m_triggerConfigOnStartup;
    }

    public void setTriggerConfigOnStartup(boolean triggerConfigOnStartup) {
        m_triggerConfigOnStartup = triggerConfigOnStartup;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    @Override
    public boolean isTrue(String id) {
        return getSetupIds().contains(id);
    }

    @Override
    public void setTrue(String id) {
        getSetupIds().add(id);
        saveSetupIds();
    }

    @Override
    public void setFalse(String id) {
        getSetupIds().remove(id);
        saveSetupIds();
    }

    @Override
    public boolean isFalse(String id) {
        return !isTrue(id);
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Override
    public Context getContext() {
        return m_context;
    }
}
