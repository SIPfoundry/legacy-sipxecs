/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
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
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationEventPublisher;
import org.springframework.context.ApplicationEventPublisherAware;

public class SipxReplicationContextImpl implements ApplicationEventPublisherAware, BeanFactoryAware,
        SipxReplicationContext {

    private static final Log LOG = LogFactory.getLog(SipxReplicationContextImpl.class);

    private BeanFactory m_beanFactory;
    private ApplicationEventPublisher m_appliationEventPublisher;
    private ReplicationManager m_replicationManager;
    private JobContext m_jobContext;
    private LocationsManager m_locationsManager;

    public void generate(DataSet dataSet) {
        Serializable jobId = m_jobContext.schedule("Data replication: " + dataSet.getName());
        boolean success = false;
        try {
            m_jobContext.start(jobId);
            LOG.info("Replicating: " + dataSet.getName());
            String beanName = dataSet.getBeanName();
            DataSetGenerator generator = (DataSetGenerator) m_beanFactory.getBean(beanName, DataSetGenerator.class);
            success = m_replicationManager.replicateData(m_locationsManager.getLocations(), generator);
        } finally {
            if (success) {
                m_jobContext.success(jobId);
            } else {
                LOG.error("Replicatiion failure: " + dataSet.getName());
                // there is not really a good info here - advise user to consult log?
                m_jobContext.failure(jobId, null, null);
            }
        }
    }

    public void generateAll() {
        for (Iterator<DataSet> i = DataSet.iterator(); i.hasNext();) {
            DataSet dataSet = i.next();
            generate(dataSet);
        }
    }

    public void replicate(ConfigurationFile file) {
        Serializable jobId = m_jobContext.schedule("File replication: " + file.getType().getName());
        boolean success = false;
        try {
            m_jobContext.start(jobId);
            success = m_replicationManager.replicateFile(m_locationsManager.getLocations(), file);
        } finally {
            if (success) {
                m_jobContext.success(jobId);
            } else {
                // there is not really a good info here - advise user to consult log?
                m_jobContext.failure(jobId, null, null);
            }
        }
    }

    public String getXml(DataSet dataSet) {
        String beanName = dataSet.getBeanName();
        DataSetGenerator generator = (DataSetGenerator) m_beanFactory.getBean(beanName, DataSetGenerator.class);
        try {
            StringWriter writer = new StringWriter();
            XMLWriter xmlWriter = new XMLWriter(writer, OutputFormat.createPrettyPrint());
            xmlWriter.write(generator.generate());
            return writer.toString();
        } catch (IOException e) {
            return "";
        }
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

    public void setApplicationEventPublisher(ApplicationEventPublisher applicationEventPublisher) {
        m_appliationEventPublisher = applicationEventPublisher;
    }

    public void publishEvent(ApplicationEvent event) {
        m_appliationEventPublisher.publishEvent(event);
    }
}
