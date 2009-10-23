/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.IOException;
import java.io.Serializable;
import java.io.StringWriter;
import java.util.Iterator;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.XMLWriter;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSetGenerator;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.ReplicationManager;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationEventPublisher;
import org.springframework.context.ApplicationEventPublisherAware;

public abstract class SipxReplicationContextImpl implements ApplicationEventPublisherAware, BeanFactoryAware,
        SipxReplicationContext {

    private static final String IGNORE_REPLICATION_MESSAGE = "In initialization phase, ignoring request to replicate ";

    private static final Log LOG = LogFactory.getLog(SipxReplicationContextImpl.class);

    private BeanFactory m_beanFactory;
    private ApplicationEventPublisher m_applicationEventPublisher;
    private ReplicationManager m_replicationManager;
    private JobContext m_jobContext;
    private LocationsManager m_locationsManager;
    // should be replicated every time aliases are replicated
    private ConfigurationFile m_validUsersConfig;

    protected abstract ServiceConfigurator getServiceConfigurator();

    public void generate(DataSet dataSet) {
        if (inInitializationPhase()) {
            LOG.debug(IGNORE_REPLICATION_MESSAGE + dataSet.getName());
            return;
        }
        String beanName = dataSet.getBeanName();
        final DataSetGenerator generator = (DataSetGenerator) m_beanFactory
                .getBean(beanName, DataSetGenerator.class);
        ReplicateWork work = new ReplicateWork() {
            public boolean replicate() {
                return m_replicationManager.replicateData(m_locationsManager.getLocations(), generator);
            }
        };
        doWithJob("Data replication: " + dataSet.getName(), work);
        // replication valid users when aliases are replicated
        if (DataSet.ALIAS.equals(dataSet)) {
            replicate(m_validUsersConfig);
        }
    }

    public void generateAll() {
        for (Iterator<DataSet> i = DataSet.iterator(); i.hasNext();) {
            DataSet dataSet = i.next();
            generate(dataSet);
        }
    }

    public void replicate(ConfigurationFile file) {
        if (inInitializationPhase()) {
            LOG.debug(IGNORE_REPLICATION_MESSAGE + file.getName());
            return;
        }

        Location[] locations = m_locationsManager.getLocations();
        replicateWorker(locations, file);
    }

    public void replicate(Location location, ConfigurationFile file) {
        if (inInitializationPhase()) {
            LOG.debug(IGNORE_REPLICATION_MESSAGE + file.getName());
            return;
        }

        Location[] locations = new Location[] {
            location
        };
        replicateWorker(locations, file);
    }

    public String getXml(DataSet dataSet) {
        String beanName = dataSet.getBeanName();
        DataSetGenerator generator = (DataSetGenerator) m_beanFactory.getBean(beanName, DataSetGenerator.class);
        try {
            StringWriter writer = new StringWriter();
            XMLWriter xmlWriter = new XMLWriter(writer, OutputFormat.createPrettyPrint());
            xmlWriter.write(generator.generateXml());
            return writer.toString();
        } catch (IOException e) {
            return "";
        }
    }

    private void replicateWorker(final Location[] locations, final ConfigurationFile file) {
        ReplicateWork work = new ReplicateWork() {
            public boolean replicate() {
                return m_replicationManager.replicateFile(locations, file);
            }

        };
        doWithJob("File replication: " + file.getName(), work);
    }

    private void doWithJob(String jobName, ReplicateWork work) {
        Serializable jobId = m_jobContext.schedule(jobName);
        boolean success = false;
        try {
            LOG.info("Start replication: " + jobName);
            m_jobContext.start(jobId);
            success = work.replicate();
        } finally {
            if (success) {
                m_jobContext.success(jobId);
            } else {
                LOG.warn("Replication failed: " + jobName);
                // there is not really a good info here - advise user to consult log?
                m_jobContext.failure(jobId, null, null);
            }
        }
    }

    private boolean inInitializationPhase() {
        String initializationPhase = System.getProperty("sipxconfig.initializationPhase");
        if (initializationPhase == null) {
            return false;
        }

        return Boolean.parseBoolean(initializationPhase);
    }

    interface ReplicateWork {
        boolean replicate();
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    @Required
    public void setReplicationManager(ReplicationManager replicationManager) {
        m_replicationManager = replicationManager;
    }

    @Required
    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setValidUsersConfig(ConfigurationFile validUsersConfig) {
        m_validUsersConfig = validUsersConfig;
    }

    @Required
    public void setApplicationEventPublisher(ApplicationEventPublisher applicationEventPublisher) {
        m_applicationEventPublisher = applicationEventPublisher;
    }

    public void publishEvent(ApplicationEvent event) {
        m_applicationEventPublisher.publishEvent(event);
    }
}
