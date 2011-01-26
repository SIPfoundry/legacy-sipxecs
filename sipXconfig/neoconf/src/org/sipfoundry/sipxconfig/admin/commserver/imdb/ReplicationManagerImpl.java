/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.io.Writer;
import java.util.Set;

import com.mongodb.BasicDBObject;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;
import com.mongodb.Mongo;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.logging.AuditLogContext;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;

public class ReplicationManagerImpl implements ReplicationManager, BeanFactoryAware {
    private static final int PERMISSIONS = 0644;
    private static final Log LOG = LogFactory.getLog(ReplicationManagerImpl.class);
    private static final String HOST = "localhost";
    private static final int PORT = 27017;
    private static final String DB_NAME = "imdb";
    private static Mongo s_mongoInstance;

    private boolean m_enabled = true;

    private ApiProvider<FileApi> m_fileApiProvider;
    private LocationsManager m_locationsManager;
    private DomainManager m_domainManager;
    private AuditLogContext m_auditLogContext;
    private BeanFactory m_beanFactory;

    static {
        if (s_mongoInstance == null) {
            try {
                s_mongoInstance = new Mongo(HOST, PORT);
            } catch (Exception e) {
                LOG.error("Unable to open mongo connection on: " + HOST + ":" + PORT);
            }
        }
    }

    public void setFileApiProvider(ApiProvider<FileApi> fileApiProvider) {
        m_fileApiProvider = fileApiProvider;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setAuditLogContext(AuditLogContext auditLogContext) {
        m_auditLogContext = auditLogContext;
    }

    public void dropDb() {
        DB datasetDb = s_mongoInstance.getDB(DB_NAME);
        DBCollection datasetCollection = datasetDb.getCollection(m_domainManager.getDomainName());
        datasetCollection.drop();
    }

    private boolean replicateData(DataSetGenerator generator) {
        if (!m_enabled) {
            return true;
        }
        boolean success = true;
        DataSet type = generator.getType();
        try {
            DB datasetDb = s_mongoInstance.getDB(DB_NAME);
            DBCollection datasetCollection = datasetDb.getCollection(m_domainManager.getDomainName());
            generator.setDbCollection(datasetCollection);
            generator.generate();
            m_auditLogContext.logReplication(type.getName(), m_locationsManager.getPrimaryLocation());
        } catch (Exception e) {
            success = false;
            LOG.error("Data replication failed: " + type.getName(), e);
        }
        return success;
    }

    @Override
    public boolean replicateAllData() {
        dropDb();
        for (DataSet dataSet : DataSet.getEnumList()) {
            String beanName = dataSet.getBeanName();
            final DataSetGenerator generator = (DataSetGenerator) m_beanFactory.getBean(beanName,
                    DataSetGenerator.class);
            replicateData(generator);
        }
        return false;
    }

    public boolean replicateEntity(Replicable entity) {
        boolean success = true;
        try {
            DB datasetDb = s_mongoInstance.getDB(DB_NAME);
            DBCollection datasetCollection = datasetDb.getCollection(m_domainManager.getDomainName());

            Set<DataSet> dataSets = entity.getDataSets();
            for (DataSet dataSet : dataSets) {
                String beanName = dataSet.getBeanName();
                final DataSetGenerator generator = (DataSetGenerator) m_beanFactory.getBean(beanName,
                        DataSetGenerator.class);
                generator.setDbCollection(datasetCollection);
                generator.generate(entity);
            }
        } catch (Exception e) {
            success = false;
            LOG.error("Data update failed: " + entity.getName(), e);
        }
        return success;
    }

    public boolean removeEntity(Replicable entity) {
        DB datasetDb = s_mongoInstance.getDB(DB_NAME);
        DBCollection datasetCollection = datasetDb.getCollection(m_domainManager.getDomainName());
        String id = DataSetGenerator.getEntityId(entity);
        DBObject search = new BasicDBObject();
        search.put(DataSetGenerator.ID, id);
        DBCursor cursor = datasetCollection.find(search);
        DBObject top = new BasicDBObject();
        if (!cursor.hasNext()) {
            top.put(DataSetGenerator.ID, id);
        } else {
            top = cursor.next();
        }
        return StringUtils.isEmpty(datasetCollection.remove(top).getError());
    }

    /**
     * Encodes payload using Base64 and returns encoded data as string
     *
     * @param payload
     * @return string representing encoded data
     */
    private String encodeBase64(byte[] payload) {
        try {
            // Base64 encoded content is always limited to US-ASCII charset
            byte[] encodedPayload = Base64.encodeBase64(payload);
            return new String(encodedPayload, "US-ASCII");
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException(e);
        }
    }

    public boolean replicateFile(Location[] locations, ConfigurationFile file) {
        if (!m_enabled) {
            return true;
        }
        boolean success = false;
        for (int i = 0; i < locations.length; i++) {
            if (!locations[i].isRegistered()) {
                continue;
            }
            if (!file.isReplicable(locations[i])) {
                LOG.info("File " + file.getName() + " cannot be replicated on location: " + locations[i].getFqdn());
                success = true;
                continue;
            }
            try {
                ByteArrayOutputStream outStream = new ByteArrayOutputStream();
                Writer writer = new OutputStreamWriter(outStream, "UTF-8");
                file.write(writer, locations[i]);
                writer.close();
                byte[] payloadBytes = outStream.toByteArray();
                String content = encodeBase64(payloadBytes);

                FileApi api = m_fileApiProvider.getApi(locations[i].getProcessMonitorUrl());
                success = api.replace(getHostname(), file.getPath(), PERMISSIONS, content);
                if (success) {
                    m_auditLogContext.logReplication(file.getName(), locations[i]);
                }
            } catch (XmlRpcRemoteException e) {
                LOG.error("File replication failed: " + file.getName(), e);
            } catch (UnsupportedEncodingException e) {
                LOG.error("UTF-8 encoding should be always supported.");
                throw new RuntimeException(e);
            } catch (IOException e) {
                LOG.error("IOException for stream writer", e);
                throw new RuntimeException(e);
            }
        }
        return success;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    private String getHostname() {
        return m_locationsManager.getPrimaryLocation().getFqdn();
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }
}
