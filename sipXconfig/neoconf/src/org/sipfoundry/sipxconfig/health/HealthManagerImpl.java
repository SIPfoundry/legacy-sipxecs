/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.health;

import java.io.Serializable;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.Stack;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

/**
 * Go to each health check provider and trigger them to check status. Right now
 * errors go to job status table but eventually they can go to dedicated page
 * that can re-check status.
 */
public class HealthManagerImpl implements HealthManager, BeanFactoryAware, SetupListener {
    private static final Log LOG = LogFactory.getLog(HealthManagerImpl.class);
    private static final String HEALTH_CHECK = "Health check";
    private Set<HealthCheckProvider> m_healthCheckProviders;
    private ListableBeanFactory m_beanFactory;
    private HealthRunner m_setupRunner;
    private JobContext m_jobContext;
    private Stack<Serializable> m_checks = new Stack<Serializable>();

    public Set<HealthCheckProvider> getHealthCheckProviders() {
        if (m_healthCheckProviders == null) {
            Map<String, HealthCheckProvider> beanMap = m_beanFactory.getBeansOfType(
                    HealthCheckProvider.class, false, false);
            m_healthCheckProviders = new HashSet<HealthCheckProvider>(beanMap.values());
        }
        return m_healthCheckProviders;
    }

    public boolean isRunning() {
        return m_setupRunner == null ? false : m_setupRunner.isAlive();
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    @Override
    public boolean setup(SetupManager manager) {
        // start thread after setup phase is finished
        if (m_setupRunner == null) {
            m_setupRunner = new HealthRunner();
            return false;
        }
        m_setupRunner.start();
        return true;
    }

    class HealthRunner extends Thread {
        @Override
        public void run() {
            for (HealthCheckProvider provider : getHealthCheckProviders()) {
                try {
                    provider.checkHealth(HealthManagerImpl.this);
                } catch (Exception e) {
                    LOG.error("non fatal exception checking health", e);
                    fail(e);
                }
            }
        }
    }

    @Override
    public void startCheck(String label) {
        startCheck(label, null);
    }

    @Override
    public void startCheck(String label, Location location) {
        Serializable job = m_jobContext.schedule(label, location);
        m_checks.push(job);
        m_jobContext.start(job);
    }

    @Override
    public void pass() {
        m_jobContext.success(m_checks.pop());
    }

    @Override
    public void fail(String message) {
        m_jobContext.failure(m_checks.pop(), HEALTH_CHECK, new HealthCheckFailure(message));
    }

    @Override
    public void fail(Exception fail) {
        m_jobContext.failure(m_checks.pop(), HEALTH_CHECK, fail);
    }

    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }
}
