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
import java.util.Map;
import java.util.Set;

import com.mongodb.BasicDBObject;
import com.mongodb.CommandResult;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;
import com.mongodb.Mongo;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.logging.AuditLogContext;
import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.ReplicableProvider;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.annotation.Required;

public class ReplicationManagerImpl implements ReplicationManager, BeanFactoryAware {
    private static final int PERMISSIONS = 0644;
    private static final Log LOG = LogFactory.getLog(ReplicationManagerImpl.class);
    private static final String HOST = "localhost";
    private static final int PORT = 27017;
    private static final String DB_NAME = "imdb";
    private static final String ENTITY_COLLECTION_NAME = "entity";
    private static final String NODE_COLLECTION_NAME = "node";
    private static final String REPLICATION_FAILED = "Replication: insert/update failed - ";
    private static final String REPLICATION_FAILED_REMOVE = "Replication: delete failed - ";
    private static final String UNABLE_OPEN_MONGO = "Unable to open mongo connection on: ";
    private static final String COLON = ":";
    private static final String ID = "id";
    private static final String IP = "ip";
    private static final String DESCRIPTION = "dsc";
    private static final String MASTER = "mstr";
    private Mongo m_mongoInstance;
    private DBCollection m_datasetCollection;

    private boolean m_enabled = true;

    private ApiProvider<FileApi> m_fileApiProvider;
    private LocationsManager m_locationsManager;
    private AuditLogContext m_auditLogContext;
    private ListableBeanFactory m_beanFactory;
    private CoreContext m_coreContext;
    private ForwardingContext m_forwardingContext;
    private ExternalAliases m_externalAliases;

    private void initMongo() throws Exception {
        if (m_mongoInstance == null) {
            try {
                m_mongoInstance = new Mongo(HOST, PORT);
                // defaults - the entity DB;
                DB datasetDb = m_mongoInstance.getDB(DB_NAME);
                m_datasetCollection = datasetDb.getCollection(ENTITY_COLLECTION_NAME);
            } catch (Exception e) {
                LOG.error(UNABLE_OPEN_MONGO + HOST + COLON + PORT);
                throw (e);
            }
        }
    }

    private Mongo initMongo(Location location) throws Exception {
        try {
            Mongo mongoInstance = new Mongo(location.getAddress(), PORT);
            return mongoInstance;
        } catch (Exception e) {
            LOG.error(UNABLE_OPEN_MONGO + HOST + COLON + PORT);
            throw (e);
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

    public void dropDb() throws Exception {
        initMongo();
        m_datasetCollection.drop();
    }

    /*
     * Get all replicable entities and replicate them; this is far better than getting the
     * DataSet.values and generating for each of them. Treat users differently since we have the
     * DaoUtils.forAllUsersDo method that should be used.
     */
    @Override
    public void replicateAllData() {
        try {
            dropDb(); // this calls initMongo()
            Closure<User> closure = new Closure<User>() {
                @Override
                public void execute(User user) {
                    replicateEntity(user);
                    CallSequence cs = m_forwardingContext.getCallSequenceForUser(user);
                    if (!cs.getRings().isEmpty()) {
                        replicateEntity(cs);
                    }
                }

            };
            DaoUtils.forAllUsersDo(m_coreContext, closure);
            Map<String, ReplicableProvider> beanMap = m_beanFactory.getBeansOfType(ReplicableProvider.class);
            for (ReplicableProvider provider : beanMap.values()) {
                for (Replicable entity : provider.getReplicables()) {
                    replicateEntity(entity);
                }
            }
            // Replicate the external aliases
            ExternalAlias extalias = new ExternalAlias();
            extalias.setFiles(m_externalAliases.getFiles());
            replicateEntity(extalias);

        } catch (Exception e) {
            LOG.error("Regeneration of database failed", e);
            throw new UserException(e);
        }
    }

    public void replicateEntity(Replicable entity) {
        try {
            initMongo();
            Set<DataSet> dataSets = entity.getDataSets();
            for (DataSet dataSet : dataSets) {
                replicateEntity(entity, dataSet);
            }
        } catch (Exception e) {
            LOG.error(REPLICATION_FAILED + entity.getName(), e);
            throw new UserException(REPLICATION_FAILED + entity.getName(), e);
        }
    }

    private void replicateEntity(Replicable entity, DataSet dataSet) {
        try {
            String beanName = dataSet.getBeanName();
            final DataSetGenerator generator = (DataSetGenerator) m_beanFactory.getBean(beanName,
                    DataSetGenerator.class);
            generator.setDbCollection(m_datasetCollection);
            generator.generate(entity);
            LOG.info("Replication: inserted/updated " + entity.getName());
        } catch (Exception e) {
            LOG.error(REPLICATION_FAILED + entity.getName(), e);
            throw new UserException(REPLICATION_FAILED + entity.getName(), e);
        }
    }

    public void replicateLocation(Location location) {
        try {
            if (location.isRegistered()) {
                initMongo();
                DB datasetDb = m_mongoInstance.getDB(DB_NAME);
                DBCollection nodeCollection = datasetDb.getCollection(NODE_COLLECTION_NAME);
                DBObject search = new BasicDBObject();
                search.put(ID, location.getId());
                DBCursor cursor = nodeCollection.find(search);
                DBObject node = new BasicDBObject();
                if (cursor.hasNext()) {
                    node = cursor.next();
                }
                node.put(ID, location.getId());
                node.put(IP, location.getAddress());
                node.put(DESCRIPTION, location.getName());
                node.put(MASTER, location.isPrimary());
                nodeCollection.save(node);
            }
        } catch (Exception e) {
            throw new UserException("Cannot register location in mongo db: " + e);
        }
    }

    public void removeEntity(Replicable entity) {
        try {
            initMongo();
            DB datasetDb = m_mongoInstance.getDB(DB_NAME);
            DBCollection datasetCollection = datasetDb.getCollection(ENTITY_COLLECTION_NAME);
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
            datasetCollection.remove(top).getError();
            LOG.info("Replication: removed " + entity.getName());
        } catch (Exception e) {
            LOG.error(REPLICATION_FAILED_REMOVE + entity.getName(), e);
            throw new UserException(REPLICATION_FAILED_REMOVE + entity.getName(), e);
        }
    }

    /**
     * Convenience method used to regenerate all users. Used in case a gateway is removed. In this
     * case we must regenerate caller alias dataset for all users. No need to regenerate call
     * sequence or other dataset
     */
    public void replicateAllUsers(final DataSet ds) {
        try {
            initMongo();
            Closure<User> closure = new Closure<User>() {
                @Override
                public void execute(User user) {
                    replicateEntity(user, ds);
                }

            };
            DaoUtils.forAllUsersDo(m_coreContext, closure);
        } catch (Exception e) {
            LOG.error("Regeneration of users failed.", e);
            throw new UserException("&user.regeneration.failed");
        }
    }

    public void removeLocation(Location location) {
        try {
            if (location.isRegistered()) {
                initMongo();
                DB datasetDb = m_mongoInstance.getDB(DB_NAME);
                DBCollection nodeCollection = datasetDb.getCollection(NODE_COLLECTION_NAME);
                DBObject search = new BasicDBObject();
                search.put(ID, location.getId());
                DBCursor cursor = nodeCollection.find(search);
                DBObject node = new BasicDBObject();
                if (cursor.hasNext()) {
                    node = cursor.next();
                }
                nodeCollection.remove(node);
            }
        } catch (Exception e) {
            throw new UserException("Cannot unregister location in mongo db: " + e);
        }
    }

    @Override
    public void resyncSlave(Location location) {
        try {
            Mongo mongo = initMongo(location);
            DB adminDb = mongo.getDB("admin");
            CommandResult cr = adminDb.command("resync");
            double ok = (Double) cr.get("ok");
            if (ok == 0) {
                LOG.error("Replication: resync - " + cr.get("errmsg"));
            } else {
                LOG.info("Replication: resync started - " + location.getAddress());
            }
        } catch (Exception e) {
            LOG.error("Replication: resync failed - " + location.getAddress(), e);
        }
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

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }

    public void setExternalAliases(ExternalAliases externalAliases) {
        m_externalAliases = externalAliases;
    }

}
