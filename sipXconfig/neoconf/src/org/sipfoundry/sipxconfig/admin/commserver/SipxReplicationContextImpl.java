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

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.Serializable;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.digester.Digester;
import org.apache.commons.digester.SetNestedPropertiesRule;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.XMLWriter;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSetGenerator;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.ReplicationManager;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigurationFile;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationEventPublisher;
import org.springframework.context.ApplicationEventPublisherAware;
import org.xml.sax.SAXException;

public class SipxReplicationContextImpl implements ApplicationEventPublisherAware,
        BeanFactoryAware, SipxReplicationContext {

    protected static final Log LOG = LogFactory.getLog(SipxProcessContextImpl.class);
    private static final String TOPOLOGY_XML = "topology.xml";
    /** these are lazily constructed - always use accessors */
    private Location[] m_locations;
    private String m_configDirectory;
    private BeanFactory m_beanFactory;
    private ApplicationEventPublisher m_appliationEventPublisher;
    private ReplicationManager m_replicationManager;
    private JobContext m_jobContext;

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    public void generate(DataSet dataSet) {
        Serializable jobId = m_jobContext.schedule("Data replication: " + dataSet.getName());
        boolean success = false;
        try {
            m_jobContext.start(jobId);
            String beanName = dataSet.getBeanName();
            DataSetGenerator generator = (DataSetGenerator) m_beanFactory.getBean(beanName,
                    DataSetGenerator.class);
            success = m_replicationManager.replicateData(getLocations(), generator);
        } finally {
            if (success) {
                m_jobContext.success(jobId);
            } else {
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
        Serializable jobId = m_jobContext.schedule("File replication: "
                + file.getType().getName());
        boolean success = false;
        try {
            m_jobContext.start(jobId);
            success = m_replicationManager.replicateFile(getLocations(), file);
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
        DataSetGenerator generator = (DataSetGenerator) m_beanFactory.getBean(beanName,
                DataSetGenerator.class);
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

    public void setReplicationManager(ReplicationManager replicationManager) {
        m_replicationManager = replicationManager;
    }

    /** Return the replication URLs, retrieving them on demand */
    public Location[] getLocations() {
        if (m_locations != null) {
            return m_locations;
        }
        try {
            InputStream stream = getTopologyAsStream();
            Digester digester = new LocationDigester();
            Collection<Location> locations = (Collection) digester.parse(stream);
            m_locations = locations.toArray(new Location[locations.size()]);
        } catch (FileNotFoundException e) {
            // When running in a test environment, the topology file will not be found
            // set to empty array so that we do not have to parse again
            m_locations = new Location[0];
            LOG.warn("Could not find the file " + TOPOLOGY_XML, e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } catch (SAXException e) {
            throw new RuntimeException(e);
        }
        return m_locations;
    }

    /** Open an input stream on the topology file and return it */
    protected InputStream getTopologyAsStream() throws FileNotFoundException {
        File file = new File(m_configDirectory, TOPOLOGY_XML);
        InputStream stream = new FileInputStream(file);
        return stream;
    }

    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }

    private static final class LocationDigester extends Digester {
        public static final String PATTERN = "topology/location";

        protected void initialize() {
            setValidating(false);
            setNamespaceAware(false);

            push(new ArrayList());
            addObjectCreate(PATTERN, Location.class);
            String[] elementNames = {
                "replication_url", "agent_url", "sip_domain"
            };
            String[] propertyNames = {
                "replicationUrl", "processMonitorUrl", "sipDomain"
            };
            addSetProperties(PATTERN);
            SetNestedPropertiesRule rule = new SetNestedPropertiesRule(elementNames,
                    propertyNames);
            // ignore all properties that we are not interested in
            rule.setAllowUnknownChildElements(true);
            addRule(PATTERN, rule);
            addSetNext(PATTERN, "add");
        }
    }

    public void setApplicationEventPublisher(ApplicationEventPublisher applicationEventPublisher) {
        m_appliationEventPublisher = applicationEventPublisher;
    }

    public void publishEvent(ApplicationEvent event) {
        m_appliationEventPublisher.publishEvent(event);
    }
}
