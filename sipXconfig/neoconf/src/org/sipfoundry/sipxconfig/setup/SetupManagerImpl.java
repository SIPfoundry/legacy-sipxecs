/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setup;

import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.jdbc.core.JdbcTemplate;

public class SetupManagerImpl implements SetupManager, ApplicationListener<ApplicationEvent>, BeanFactoryAware {
    private ListableBeanFactory m_beanFactory;
    private Set<String> m_setupIds;
    private ConfigManager m_configManager;
    private FeatureManager m_featureManager;
    private JdbcTemplate m_jdbcTemplate;
    private Set<SetupListener> m_setupListeners;
    private boolean m_dirty;
    private boolean m_enabled = true;

    @Override
    public boolean isSetup(String id) {
        return getSetupIds().contains(id);
    }

    @Override
    public void setSetup(String id) {
        m_dirty = true;
        getSetupIds().add(id);
    }

    Set<String> getSetupIds() {
        if (m_setupIds == null) {
            List<String> setupIds = m_jdbcTemplate.queryForList("select setup_id from setup", String.class);
            m_setupIds = new HashSet<String>(setupIds);
        }
        return m_setupIds;
    }

    void saveSetupIds() {
        if (m_dirty) {
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
            m_dirty = false;
        }
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

    @Override
    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof ApplicationInitializedEvent && m_enabled) {
            for (SetupListener l : getSetupListeners()) {
                l.setup(this);
            }
            saveSetupIds();
        }
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
}
